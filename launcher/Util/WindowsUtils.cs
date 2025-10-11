using System;
using System.IO;
using System.Net.Http;
using System.Runtime.Versioning;
using System.Threading.Tasks;
 
using CNC.NCO.Launcher.Config;

namespace CNC.NCO.Launcher.Util;

[SupportedOSPlatform("windows")]
public class WindowsUtils(PathsConfig paths)
{
  private static readonly string StartMenuPath = Path.Join(
    Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
    "Microsoft",
    "Windows",
    "Start Menu",
    "Programs"
  );

  public async Task CreateShortcut(string shortcutName, string targetPath)
  {
    try
    {
      var shortcutPath = Path.Join(StartMenuPath, shortcutName);

      var startInfo = ProcessUtils.BuildStartInfo(
        "powershell",
        @"-ExecutionPolicy Bypass",
        @"-File create-or-update-shortcut.ps1",
        $"-ShortcutPath '{shortcutPath}'",
        $"-TargetPath '{targetPath}'"
      );
      startInfo.WorkingDirectory = paths.ToolsPath;

      (await ProcessUtils.Exec(startInfo))
        .AssertExitCode($"PowerShell script 'create-or-update-shortcut.ps1' failed", 2);
    }
    catch (Exception ex)
    {
      throw new Exception($"Error creating shortcut: {shortcutName}", ex);
    }
  }

  public async Task InstallMsvcRuntime(string msvcInstallerUrl)
  {
    try
    {
      // fetch runtime installer
      var tempFile = Path.ChangeExtension(Path.GetTempFileName(), ".exe");

      using var client = new HttpClient();
      var response = await client.GetAsync(msvcInstallerUrl);

      response.EnsureSuccessStatusCode();

      await using (var fileStream = new FileStream(tempFile, FileMode.Create))
      {
        await response.Content.CopyToAsync(fileStream);
      }

      // exec runtime installer
      var startInfo = ProcessUtils.BuildStartInfo(
        tempFile,
        "/install",
        "/quiet",
        "/norestart"
      );
      startInfo.CreateNoWindow = false;
      startInfo.UseShellExecute = true;
      startInfo.Verb = "runas";

      var execResult = await ProcessUtils.Exec(startInfo);

      File.Delete(tempFile);

      execResult.AssertExitCode("MSVC redistributable exec failed");
    }
    catch (Exception e)
    {
      throw new Exception("MSVC runtime failed to install due to an error", e);
    }
  }
}
