using System;
using System.IO;

namespace CNC.NCO.Launcher.Config;

public class PathsConfig(
  string launcherDirectoryPath,
  string appDataDirectoryPath,
  string userHomePath,
  string userDesktopPath
)
{
  // launcher app paths
  public string LauncherDirectoryPath => launcherDirectoryPath;
  public string ConfigYamlPath { get; } = Path.Join(launcherDirectoryPath, "config.yml");

#if DEBUG
  // ensure native tools can be resolved in a debug build (no runtime identifier present)
  public string ToolsPath { get; } = Path.Join(
    launcherDirectoryPath,
    "tools",
    "runtimes",
    OperatingSystem.IsWindows()
      ? "win-x64"
      : OperatingSystem.IsMacOS() ? "osx" : "linux-x64",
    "native"
  );
#else
  public string ToolsPath { get; } = Path.Join(launcherDirectoryPath, "tools");
#endif

  // user paths
  public string UserHomePath => userHomePath;
  public string UserDesktopPath => userDesktopPath;
  public string AppDataDirectoryPath => appDataDirectoryPath;
  public string NcoAppDataPath =>
    OperatingSystem.IsMacOS()
    ? Path.Join(UserHomePath, "Library/Application Support/NCO")
    : Path.Join(appDataDirectoryPath, "nco");

  public string NcoCachePath => Path.Join(NcoAppDataPath, ".cache");
  public string UserConfigYamlPath => Path.Join(NcoAppDataPath, "config.yml");
}
