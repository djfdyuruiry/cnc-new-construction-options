using System;
using System.IO;
using System.Linq;
using System.Threading.Tasks;

using DiscUtils.Iso9660;
using InstallShieldExtractor;

using CNC.NCO.Launcher.Model;
using CNC.NCO.Launcher.Model.Events.Download;

namespace CNC.NCO.Launcher.Util;

public static class GameDiscUtils
{
  private static async Task<Stream?> GetStreamForSetupPackageFile(
    CDReader iso,
    DiscImageSource source,
    string fileName
  )
  {
    if (source.HasSetupPackageFile || !iso.FileExists(source.SetupPackageFile))
    {
      return null;
    }

    await using var setupStream = iso.OpenFile(source.SetupPackageFile, FileMode.Open);
    using var setupPackage = new InstallShieldPackage(setupStream, source.SetupPackageFile!.Split(@"\").Last());

    return setupPackage.Contents
      .Where(p => p.EndsWith(fileName))
      .Select(p => setupPackage.GetStream(p))
      .FirstOrDefault();
  }

  public static async Task ExtractFile(
    CDReader iso,
    IDownloadEventVisitor downloadEventVisitor,
    DiscImageSource isoSource,
    string isoOrSetupPath,
    string outputPath
  )
  {
    try
    {
      Console.WriteLine($"Extracting {isoOrSetupPath} to {outputPath}");

      if (isoSource.HasSetupPackageFile && !iso.FileExists(isoSource.SetupPackageFile))
      {
        throw new Exception($"ISO image does not contain required setup package file: {isoSource.SetupPackageFile}");
      }

      var isIsoFile = iso.FileExists(isoOrSetupPath);

      await using var sourceFileStream = isIsoFile
        ? iso.OpenFile(isoOrSetupPath, FileMode.Open)
        : await GetStreamForSetupPackageFile(iso, isoSource, isoOrSetupPath);

      if (sourceFileStream is null)
      {
        throw new Exception(
          isIsoFile
            ? "File not found in ISO image"
            : $"File not found in ISO setup package file: {isoSource.SetupPackageFile}"
        );
      }

      await using var outputStream = File.Open(outputPath, FileMode.Create);
      await sourceFileStream.CopyToAsync(outputStream);

      downloadEventVisitor.Visit(new WriteGameDataFileEvent(isoOrSetupPath, outputPath));
    }
    catch (Exception ex)
    {
      throw new Exception($"Error extracting {isoOrSetupPath} to {outputPath}", ex);
    }
  }
}
