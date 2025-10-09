using System;
using System.Diagnostics;
using System.IO;
using System.Net.Http;
using System.Runtime.Versioning;
using System.Threading.Tasks;

using CNC.NCO.Launcher.Config;

namespace CNC.NCO.Launcher.Util;

[SupportedOSPlatform("windows")]
public class WindowsUtils(PathsConfig paths)
{
  public async Task CreateShortcut(string shortcutName, string targetPath)
  {
    try
    {
      var powershellScriptPath = Path.Join(paths.LauncherDirectoryPath, "create-or-update-shortcut.ps1");
      var shortcutPath = Path.Join(
        Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
        "Microsoft",
        "Windows",
        "Start Menu",
        "Programs",
        shortcutName
      );

      using var powershellProcess = Process.Start(new ProcessStartInfo
      {
        FileName = "powershell",
        Arguments = string.Join(
          " ",
          "-ExecutionPolicy Bypass",
          $"-File '{powershellScriptPath}'",
          $"-ShortcutName '{shortcutPath}'",
          $"-TargetPath '{targetPath}'"
        ),
        UseShellExecute = false,
        CreateNoWindow = true
      });

      if (powershellProcess is null)
      {
        throw new Exception($"Failed to start powershell process");
      }

      await powershellProcess.WaitForExitAsync();

      if (powershellProcess.ExitCode != 0)
      {
        throw new Exception($"PowerShell script failed with exit code {powershellProcess.ExitCode}");
      }
    }
    catch (Exception ex)
    {
      Console.WriteLine($"Error creating shortcut: {shortcutName}", ex);
    }
  }

  public async Task InstallMsvcRuntime(string msvcInstallerUrl)
  {
    var tempFile = Path.ChangeExtension(Path.GetTempFileName(), ".exe");

    using var client = new HttpClient();
    var response = await client.GetAsync(msvcInstallerUrl);

    response.EnsureSuccessStatusCode();

    await using (var fileStream = new FileStream(tempFile, FileMode.Create))
    {
      await response.Content.CopyToAsync(fileStream);
    }

    using var msvcInstaller = Process.Start(
      new ProcessStartInfo
      {
        FileName = tempFile,
        Arguments = "/install /quiet /norestart",
        UseShellExecute = true,
        Verb = "runas"
      }
    );

    if (msvcInstaller is null || msvcInstaller.HasExited)
    {
      throw new Exception($"Failed to install MSVC redistributable, exit code: {msvcInstaller?.ExitCode ?? -1}");
    }

    await msvcInstaller.WaitForExitAsync();

    File.Delete(tempFile);

    if (msvcInstaller.ExitCode != 0)
    {
      throw new Exception($"Failed to install MSVC redistributable, exit code: {msvcInstaller.ExitCode}");
    }
  }
}
