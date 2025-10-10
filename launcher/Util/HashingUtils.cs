using System;
using System.IO;

using Blake3;

namespace CNC.NCO.Launcher.Util;

public class HashingUtils
{
  public static string CalculateFileChecksum(string filename)
  {
    using var stream = File.OpenRead(filename);
    using var blake3Stream = new Blake3Stream(stream);

    return Convert.ToHexString(
      blake3Stream.ComputeHash().AsSpan()
    );
  }

  public static bool FileChecksumIsValid(
    string destinationPath,
    string expectedChecksum,
    bool deleteInvalidFiles = false
  )
  {
    if (!File.Exists(destinationPath))
    {
      return false;
    }

    var isValid = CalculateFileChecksum(destinationPath) == expectedChecksum;

    if (!isValid)
    {
      // cleanup corrupt file
      File.Delete(destinationPath);
    }

    return isValid;
  }

}
