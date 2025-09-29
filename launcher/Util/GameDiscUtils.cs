using System;
using System.IO;
using System.Linq;
using System.Threading.Tasks;

using DiscUtils.Iso9660;
using InstallShieldExtractor;

namespace CNC.NCO.Launcher.Util;

public class GameDiscUtils
{
  private static async Task<Stream?> GetStreamForSetupPackageFile(CDReader iso, string name)
  {
    await using var setupStream = iso.OpenFile(@"INSTALL\SETUP.Z", FileMode.Open);
    using var setupPackage = new InstallShieldPackage(setupStream, "SETUP.Z");

    return setupPackage.Contents
      .Where(p => p.EndsWith(name)).Select(p => setupPackage.GetStream(p))
      .FirstOrDefault();
  }

  public static async Task ExtractFile(CDReader iso, string isoOrSetupPath, string outputPath)
  {
    try
    {
      Console.WriteLine($"Extracting {isoOrSetupPath} to {outputPath}");

      await using var sourceFileStream = iso.FileExists(isoOrSetupPath)
        ? iso.OpenFile(isoOrSetupPath, FileMode.Open)
        : (await GetStreamForSetupPackageFile(iso, isoOrSetupPath))!;

      await using var outputStream = File.Open(outputPath, FileMode.Create);
      await sourceFileStream.CopyToAsync(outputStream);
    }
    catch (Exception ex)
    {
      Console.WriteLine($"Error extracting {isoOrSetupPath} to {outputPath}: {ex.Message}");
    }
  }

}
