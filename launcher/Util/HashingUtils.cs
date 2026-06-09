using System;
using System.IO;
using System.Linq;
using System.Runtime.Versioning;
using System.Threading.Tasks;

using Blake3;

namespace CNC.NCO.Launcher.Util;

[SupportedOSPlatform("linux")]
[SupportedOSPlatform("windows")]
[SupportedOSPlatform("osx")]
public static class HashingUtils
{
  public static async Task<Hash> CalculateFileChecksum(string path, int bufferSize = 4096 * 32)
  {
    using var hasher = Hasher.New();
    await using var stream = File.OpenRead(path);

    var buffer = new byte[bufferSize];
    int bytesRead;

    while ((bytesRead = await stream.ReadAsync(buffer)) != 0)
    {
      hasher.Update(buffer.AsSpan(start: 0, bytesRead));
    }

    return hasher.Finalize();
  }

  public static async Task<bool> FileChecksumIsValid(
    string filePath,
    string expectedChecksum,
    bool deleteInvalidFiles = false
  )
  {
    if (!File.Exists(filePath))
    {
      return false;
    }

    var fileHash = await CalculateFileChecksum(filePath);
    var isValid = string.Equals(
      fileHash.ToString(),
      expectedChecksum,
      StringComparison.OrdinalIgnoreCase
    );

    if (!isValid && deleteInvalidFiles)
    {
      // cleanup corrupt file
      await Console.Error.WriteLineAsync($"WARN Purging corrupted file due to mismatched checksum: {filePath}");
      File.Delete(filePath);
    }

    return isValid;
  }

  public static async Task<Hash> CalculateDirectoryChecksum(
    string path,
    string searchPattern,
    int bufferSize = 4096 * 32
  )
  {
    using var hasher = Hasher.New();

    foreach (var matchedFile in Directory.GetFiles(path, searchPattern).Order())
    {
      await using var stream = File.OpenRead(matchedFile);

      var buffer = new byte[bufferSize];
      int bytesRead;

      while ((bytesRead = await stream.ReadAsync(buffer)) != 0)
      {
        hasher.Update(buffer.AsSpan(start: 0, bytesRead));
      }
    }

    return hasher.Finalize();
  }

  public static async Task<bool> DirectoryChecksumIsValid(
    string path,
    string searchPattern,
    string expectedChecksum,
    bool deleteInvalidFiles = false
  )
  {
    if (!Directory.Exists(path))
    {
      return false;
    }

    var directoryHash = await CalculateDirectoryChecksum(path, searchPattern);
    var isValid = string.Equals(
      directoryHash.ToString(),
      expectedChecksum,
      StringComparison.OrdinalIgnoreCase
    );

    if (!isValid && deleteInvalidFiles)
    {
      // cleanup corrupt directory
      await Console.Error.WriteLineAsync($"WARN Purging corrupted directory due to mismatched checksum: {path}");
      Directory.Delete(path, true);
    }

    return isValid;
  }
}
