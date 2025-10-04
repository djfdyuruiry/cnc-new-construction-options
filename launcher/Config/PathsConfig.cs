using System.IO;

namespace CNC.NCO.Launcher.Config;

public class PathsConfig(string launcherDirectoryPath, string appDataDirectoryPath)
{
  // launcher app paths
  public string ConfigYamlPath { get; } = Path.Join(launcherDirectoryPath, "config.yml");
  public string ToolsPath { get; } = Path.Join(launcherDirectoryPath, "tools");
  
  // user paths
  public string AppDataPath { get; } = Path.Join(appDataDirectoryPath, "nco");
  public string CachePath => Path.Join(AppDataPath, ".cache");
  public string UserConfigYamlPath => Path.Join(AppDataPath, "config.yml");
}
