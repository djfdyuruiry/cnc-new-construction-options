using System;
using System.IO;

using YamlDotNet.Serialization;

using CNC.NCO.Launcher.Model;

namespace CNC.NCO.Launcher.Config;

public class LauncherConfigService
{
  private readonly PathsConfig _paths;

  public LauncherConfig Config { get; private set;  }

  public LauncherConfigService(PathsConfig paths)
  {
    _paths = paths;
    RefreshFromUserOrDefaultConfig();
  }

  private LauncherConfig LoadFromFile(string filePath)
  {
    try
    {
      var yaml = File.ReadAllText(filePath);

      return new DeserializerBuilder()
        .Build()
        .Deserialize<LauncherConfig>(yaml);
    }
    catch (Exception ex)
    {
      throw new InvalidOperationException(
        $"Failed to load configuration from {filePath}",
        ex
      );
    }
  }

  public void RefreshFromUserOrDefaultConfig()
  {
    try
    {
      // Try to load from app data directory first
      Config = LoadFromFile(
        File.Exists(_paths.UserConfigYamlPath) ? _paths.UserConfigYamlPath : _paths.ConfigYamlPath
      );
    }
    catch (Exception ex)
    {
      throw new InvalidOperationException(
        $"Failed to load configuration from {_paths.ConfigYamlPath} or {_paths.UserConfigYamlPath}",
        ex
      );
    }
  }

  public void SaveUserConfig()
  {
    try
    {
      Directory.CreateDirectory(
        Path.GetDirectoryName(_paths.UserConfigYamlPath)!
      );

      var yaml = new SerializerBuilder()
        .Build()
        .Serialize(Config);

      File.WriteAllText(_paths.UserConfigYamlPath, yaml);
    }
    catch (Exception ex)
    {
      throw new InvalidOperationException(
        $"Failed to save configuration to {_paths.UserConfigYamlPath}",
        ex
      );
    }
  }
}
