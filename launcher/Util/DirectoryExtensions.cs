using System.IO;

namespace CNC.NCO.Launcher.Util;

public static class DirectoryUtils
{
  public static bool CreateDirectoryIfMissing(string path)
  {
    if (Directory.Exists(path))
    {
      return false;
    }

    Directory.CreateDirectory(path);
    return true;
  }
}
