using System;
using System.IO;
using System.Threading;

using YamlDotNet.Serialization;

using CNC.NCO.Launcher.Model;

namespace CNC.NCO.Launcher.Config;

public class LauncherConfigService
{
  private static readonly Mutex SetConfigMutex = new();

  private readonly PathsConfig _paths;
  private LauncherConfig? _config;

  /// <summary>
  /// <para>
  /// - Lazy loaded thread-safe shared config instance intended to
  /// be used throughout the lifetime of a service instance
  /// </para>
  /// - Ensures any issues with YAML deserialization only bubble up
  /// when properties in LauncherConfig are required
  /// <para>
  /// - Do not access this directly in a constructor, reference in methods
  /// or callbacks instead
  /// </para>
  /// </summary>
  public LauncherConfig Config
  {
    get
    {
      if (_config is not null)
      {
        return _config;
      }

      try
      {
        SetConfigMutex.WaitOne();

        return _config ??= LoadFromUserOrDefaultConfig();
      }
      finally
      {
        SetConfigMutex.ReleaseMutex();
      }
    }
  }

  public LauncherConfigService(PathsConfig paths)
  {
    _paths = paths;
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
      throw new Exception(
        $"Failed to load configuration from {filePath}",
        ex
      );
    }
  }

  public LauncherConfig LoadFromUserOrDefaultConfig()
  {
    try
    {
      // Try to load from app data directory first
      return LoadFromFile(
        File.Exists(_paths.UserConfigYamlPath) ? _paths.UserConfigYamlPath : _paths.ConfigYamlPath
      );
    }
    catch (Exception ex)
    {
      throw new Exception(
        $"Failed to load configuration from {_paths.ConfigYamlPath} or {_paths.UserConfigYamlPath}",
        ex
      );
    }
  }

  public void SaveUserConfig()
  {
    if (_config is null)
    {
      throw new InvalidOperationException("Attempted to save user config, but it has not been loaded yet");
    }

    try
    {
      Directory.CreateDirectory(
        Path.GetDirectoryName(_paths.UserConfigYamlPath)!
      );

      var yaml = new SerializerBuilder()
        .Build()
        .Serialize(_config);

      File.WriteAllText(_paths.UserConfigYamlPath, yaml);
    }
    catch (Exception ex)
    {
      throw new Exception(
        $"Failed to save configuration to {_paths.UserConfigYamlPath}",
        ex
      );
    }
  }
}
