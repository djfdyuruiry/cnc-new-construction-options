using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using CNC.NCO.Launcher.Config;

namespace CNC.NCO.Launcher.Service;

public class Bin2IsoService(PathsConfig paths)
{
  private string OsBin2IsoPath => Path.Join(
    paths.ToolsPath,
    OperatingSystem.IsWindows() 
      ? "bin2iso.exe"
      : (OperatingSystem.IsMacOS() ? "bin2iso-macos" : "bin2iso-linux")
  );

  public async Task<int> CallBin2Iso(string[] cliParameters)
  {
    var convertToCueProc = Process.Start(OsBin2IsoPath, cliParameters);

    await convertToCueProc.WaitForExitAsync();

    return convertToCueProc.ExitCode;
  }

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
    if (await CallBin2Iso([ cuePath, "-c", binImagePath ]) != 0)
    {
      throw new Exception($"Failed to generate cue from bin file: {binImagePath}");
    }

    // extract data track from bin/cue image
    if (await CallBin2Iso([ cuePath, isoImageDirectory, "-t", "1" ]) != 0)
    {
      throw new Exception($"Failed to extract iso from bin file: {binImagePath}");
    }

    // find data track iso output file 
    return Directory.GetFileSystemEntries(Path.GetDirectoryName(cuePath)!)
      .First(f => 
        Path.GetExtension(f).Equals(".iso", StringComparison.InvariantCultureIgnoreCase)
        && Path.GetFileName(f).StartsWith(isoFileNoExtensions, StringComparison.InvariantCultureIgnoreCase)
      );
  }
}
