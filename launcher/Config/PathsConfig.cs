using System.IO;

namespace CNC.NCO.Launcher.Config;

public class PathsConfig(string baseDirectoryPath)
{
  public string ConfigYamlPath { get; } = Path.Join(baseDirectoryPath, "config.yml");
  public string ToolsPath { get; } = Path.Join(baseDirectoryPath, "tools");
  public string CachePath { get; } = Path.Join(baseDirectoryPath, ".cache");
}
