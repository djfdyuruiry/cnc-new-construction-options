using System;
using System.Collections.Generic;
using System.IO;

namespace InstallShieldExtractor
{
    public class InstallShieldPackage : IDisposable
    {
        public readonly record struct Entry(uint Offset, uint Length);

        public string Name { get; }
        public IEnumerable<string> Contents => index.Keys;

        private Dictionary<string, Entry> index;
        private Stream s;
        private long dataStart = 255;

        public InstallShieldPackage(Stream s, string filename)
        {
            Name = filename;
            this.s = s;

            try
            {
                // Parse package header
                s.ReadUInt32(); // signature
                s.Position += 8;
                s.ReadUInt16(); // FileCount
                s.Position += 4;
                s.ReadUInt32(); // ArchiveSize
                s.Position += 19;
                var tocAddress = s.ReadInt32();
                s.Position += 4;
                var dirCount = s.ReadUInt16();

                // Parse the directory list
                s.Position = tocAddress;

                // Parse directories
                var directories = new Dictionary<string, uint>(dirCount);
                var totalFileCount = 0;
                for (var i = 0; i < dirCount; i++)
                {
                    // Parse directory header
                    var fileCount = s.ReadUInt16();
                    var chunkSize = s.ReadUInt16();
                    var nameLength = s.ReadUInt16();
                    var dirName = s.ReadASCII(nameLength);

                    // Skip to the end of the chunk
                    s.Position += chunkSize - nameLength - 6;
                    directories.Add(dirName, fileCount);
                    totalFileCount += fileCount;
                }

                // Parse files
                index = new Dictionary<string, Entry>(totalFileCount);
                foreach (var dir in directories)
                    for (var i = 0; i < dir.Value; i++)
                        ParseFile(dir.Key);

                index.TrimExcess();
            }
            catch
            {
                Dispose();
                throw;
            }
        }

        uint accumulatedData = 0;
        void ParseFile(string dirName)
        {
            s.Position += 7;
            var compressedSize = s.ReadUInt32();
            s.Position += 12;
            var chunkSize = s.ReadUInt16();
            s.Position += 4;
            var nameLength = s.ReadUInt8();
            var fileName = dirName + "\\" + s.ReadASCII(nameLength);

            // Use index syntax to overwrite any duplicate entries with the last value
            index[fileName] = new Entry(accumulatedData, compressedSize);
            accumulatedData += compressedSize;

            // Skip to the end of the chunk
            s.Position += chunkSize - nameLength - 30;
        }

        public Stream? GetStream(string filename)
        {
            if (!index.TryGetValue(filename, out var e))
                return null;

            s.Seek(dataStart + e.Offset, SeekOrigin.Begin);

            var ret = new MemoryStream();
            Blast.Decompress(s, ret);
            ret.Seek(0, SeekOrigin.Begin);

            return ret;
        }

        public bool Contains(string filename)
        {
            return index.ContainsKey(filename);
        }

        public void Dispose()
        {
            GC.SuppressFinalize(this);
            s.Dispose();
        }
    }

    internal static class Blast
    {
        public const int MAXBITS = 13; // maximum code length
        public const int MAXWIN = 4096; // maximum window size

        static readonly byte[] LitLen =
        [
            11, 124, 8, 7, 28, 7, 188, 13, 76, 4,
            10, 8, 12, 10, 12, 10, 8, 23, 8, 9,
            7, 6, 7, 8, 7, 6, 55, 8, 23, 24,
            12, 11, 7, 9, 11, 12, 6, 7, 22, 5,
            7, 24, 6, 11, 9, 6, 7, 22, 7, 11,
            38, 7, 9, 8, 25, 11, 8, 11, 9, 12,
            8, 12, 5, 38, 5, 38, 5, 11, 7, 5,
            6, 21, 6, 10, 53, 8, 7, 24, 10, 27,
            44, 253, 253, 253, 252, 252, 252, 13, 12, 45,
            12, 45, 12, 61, 12, 45, 44, 173
        ];

        // bit lengths of length codes 0..15
        static readonly byte[] LenLen = [2, 35, 36, 53, 38, 23];

        // bit lengths of distance codes 0..63
        static readonly byte[] DistLen = [2, 20, 53, 230, 247, 151, 248];

        // base for length codes
        static readonly short[] LengthBase =
        [
            3, 2, 4, 5, 6, 7, 8, 9, 10, 12,
            16, 24, 40, 72, 136, 264
        ];

        // extra bits for length codes
        static readonly byte[] Extra =
        [
            0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
            3, 4, 5, 6, 7, 8
        ];

        static readonly Huffman LitCode = new(LitLen, 256);
        static readonly Huffman LenCode = new(LenLen, 16);
        static readonly Huffman DistCode = new(DistLen, 64);

        /// <summary>PKWare Compression Library stream.</summary>
        /// <param name="input">Compressed input stream.</param>
        /// <param name="output">Stream to write the decompressed output.</param>
        public static void Decompress(Stream input, Stream output)
        {
            var br = new BitReader(input);

            // Are literals coded?
            var coded = br.ReadBits(8);

            if (coded < 0 || coded > 1)
                throw new NotImplementedException("Invalid data stream");
            var encodedLiterals = coded == 1;

            // log2(dictionary size) - 6
            var dict = br.ReadBits(8);
            if (dict < 4 || dict > 6)
                throw new InvalidDataException("Invalid dictionary size");

            // output state
            ushort next = 0; // index of next write location in out[]
            var first = true; // true to check distances (for first 4K)
            var outBuffer = new byte[MAXWIN]; // output buffer and sliding window

            // decode literals and length/distance pairs
            while (true)
            {
                // length/distance pair
                if (br.ReadBits(1) == 1)
                {
                    // Length
                    var symbol = Decode(LenCode, br);
                    var len = LengthBase[symbol] + br.ReadBits(Extra[symbol]);

                    // Magic number for "done"
                    if (len == 519)
                    {
                        for (var i = 0; i < next; i++)
                            output.WriteByte(outBuffer[i]);
                        break;
                    }

                    // Distance
                    symbol = len == 2 ? 2 : dict;
                    var dist = Decode(DistCode, br) << symbol;
                    dist += br.ReadBits(symbol);
                    dist++;

                    if (first && dist > next)
                        throw new InvalidDataException("Attempt to jump before data");

                    // copy length bytes from distance bytes back
                    while (len != 0)
                    {
                        var dest = next;
                        var source = dest - dist;

                        var copy = MAXWIN;
                        if (next < dist)
                        {
                            source += copy;
                            copy = dist;
                        }

                        copy -= next;
                        if (copy > len)
                            copy = len;

                        len -= copy;
                        next += (ushort)copy;

                        // copy with old-fashioned memcpy semantics
                        while (copy-- > 0)
                            outBuffer[dest++] = outBuffer[source++];

                        // Flush window to outstream
                        if (next == MAXWIN)
                        {
                            for (var i = 0; i < next; i++)
                                output.WriteByte(outBuffer[i]);
                            next = 0;
                            first = false;
                        }
                    }
                }
                else
                {
                    // literal value
                    var symbol = encodedLiterals ? Decode(LitCode, br) : br.ReadBits(8);
                    outBuffer[next++] = (byte)symbol;
                    if (next == MAXWIN)
                    {
                        for (var i = 0; i < next; i++)
                            output.WriteByte(outBuffer[i]);
                        next = 0;
                        first = false;
                    }
                }
            }
        }

        // Decode a code using Huffman table h.
        static int Decode(Huffman h, BitReader br)
        {
            var code = 0; // len bits being decoded
            var first = 0; // first code of length len
            var index = 0; // index of first code of length len in symbol table
            short next = 1;
            while (true)
            {
                code |= br.ReadBits(1) ^ 1; // invert code
                int count = h.Count[next++];
                if (code < first + count)
                    return h.Symbol[index + (code - first)];

                index += count;
                first += count;
                first <<= 1;
                code <<= 1;
            }
        }

        sealed class BitReader
        {
            readonly Stream stream;
            byte bitBuffer = 0;
            int bitCount = 0;

            public BitReader(Stream stream)
            {
                this.stream = stream;
            }

            public int ReadBits(int count)
            {
                var ret = 0;
                for (var filled = 0; filled < count; filled++)
                {
                    if (bitCount == 0)
                    {
                        bitBuffer = stream.ReadUInt8();
                        bitCount = 8;
                    }

                    ret |= (bitBuffer & 1) << filled;
                    bitBuffer >>= 1;
                    bitCount--;
                }

                return ret;
            }
        }

        sealed class Huffman
        {
            public short[] Count; // number of symbols of each length
            public short[] Symbol; // canonically ordered symbols

            public Huffman(byte[] rep, short symbolCount)
            {
                var length = new short[256]; // code lengths
                var s = 0; // current symbol

                // convert compact repeat counts into symbol bit length list
                foreach (var code in rep)
                {
                    var num = (code >> 4) + 1; // Number of codes (top four bits plus 1)
                    var len = (byte)(code & 15); // Code length (low four bits)
                    do
                        length[s++] = len;
                    while (--num > 0);
                }

                var n = s;

                // count number of codes of each length
                Count = new short[MAXBITS + 1];
                for (var i = 0; i < n; i++)
                    Count[length[i]]++;

                // no codes!
                if (Count[0] == n)
                    return;

                // check for an over-subscribed or incomplete set of lengths
                var left = 1; // one possible code of zero length
                for (var len = 1; len <= MAXBITS; len++)
                {
                    left <<= 1; // one more bit, double codes left
                    left -= Count[len]; // deduct count from possible codes
                    if (left < 0)
                        throw new InvalidDataException("over subscribed code set");
                }

                // generate offsets into symbol table for each length for sorting
                var offs = new short[MAXBITS + 1];
                for (var len = 1; len < MAXBITS; len++)
                    offs[len + 1] = (short)(offs[len] + Count[len]);

                // put symbols in table sorted by length, by symbol order within each length
                Symbol = new short[symbolCount];
                for (short i = 0; i < n; i++)
                    if (length[i] != 0)
                        Symbol[offs[length[i]]++] = i;
            }
        }
    }

    internal static class StreamExtensions
    {
        public static uint ReadUInt32(this Stream stream)
        {
            var buffer = new byte[4];
            stream.Read(buffer, 0, 4);
            return BitConverter.ToUInt32(buffer, 0);
        }

        public static ushort ReadUInt16(this Stream stream)
        {
            var buffer = new byte[2];
            stream.Read(buffer, 0, 2);
            return BitConverter.ToUInt16(buffer, 0);
        }

        public static uint ReadInt32(this Stream stream)
        {
            var buffer = new byte[4];
            stream.Read(buffer, 0, 4);
            return (uint)BitConverter.ToInt32(buffer, 0);
        }

        public static byte ReadUInt8(this Stream stream)
        {
            var buffer = new byte[1];
            stream.Read(buffer, 0, 1);
            return buffer[0];
        }

        public static string ReadASCII(this Stream stream, int length)
        {
            var buffer = new byte[length];
            stream.Read(buffer, 0, length);
            return System.Text.Encoding.ASCII.GetString(buffer);
        }
    }
}
