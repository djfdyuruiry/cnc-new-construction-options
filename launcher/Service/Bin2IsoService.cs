using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Model.Util;
using CNC.NCO.Launcher.Util;

namespace CNC.NCO.Launcher.Service;

public class Bin2IsoService(PathsConfig paths) : IDisposable
{
  private readonly List<string> _generatedIsoImages = [];

  private string OsBin2IsoPath => Path.Join(
    paths.ToolsPath,
    OperatingSystem.IsWindows() ? "bin2iso.exe" : "bin2iso"
  );

  public async Task<ProcessExecuteResult> CallBin2Iso(params string[] cliParameters) =>
    await ProcessUtils.Exec(OsBin2IsoPath, cliParameters);

  public async Task<string> ConvertBinToIso(string binImagePath)
  {
    var isoImageDirectory = Path.GetDirectoryName(binImagePath)!;
    var isoFileNoExtensions = Path.GetFileNameWithoutExtension(binImagePath);
    var cuePath = Path.ChangeExtension(binImagePath, "cue");

    if (File.Exists(cuePath))
    {
      File.Delete(cuePath);
    }

    // generate cue file from bin image (ignore existing)
    var generateCueResult = await CallBin2Iso(cuePath, "-c", binImagePath);
    generateCueResult.AssertExitCode($"Failed to generate cue from bin file: {binImagePath}");

    // extract data track from bin/cue image
    var extractCueTrackResult = await CallBin2Iso(cuePath, isoImageDirectory, "-t", "1");
    extractCueTrackResult.AssertExitCode($"Failed to extract iso from bin file: {binImagePath}");

    // find data track iso output file 
    var isoImage = Directory.GetFileSystemEntries(Path.GetDirectoryName(cuePath)!)
      .First(f =>
        Path.GetExtension(f).Equals(".iso", StringComparison.OrdinalIgnoreCase)
        && Path.GetFileName(f).StartsWith(isoFileNoExtensions, StringComparison.OrdinalIgnoreCase)
      );

    _generatedIsoImages.Add(isoImage);

    return isoImage;
  }

  public void Dispose()
  {
    foreach (var image in _generatedIsoImages.Where(File.Exists))
    {
      try
      {
        File.Delete(image);
      }
      catch (Exception ex) 
      {
        Console.Error.WriteLine($"Failed to cleanup generated iso file '{image}': {ex}");
      };
    }

    GC.SuppressFinalize(this);
  }
}
