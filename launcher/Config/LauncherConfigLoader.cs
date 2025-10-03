using System;
using System.IO;

using YamlDotNet.Serialization;

using CNC.NCO.Launcher.Model;

namespace CNC.NCO.Launcher.Config;

public class LauncherConfigLoader(PathsConfig paths)
{
  public LauncherConfig Load()
  {
    try
    {
      return new DeserializerBuilder()
        .Build()
        .Deserialize<LauncherConfig>(
          File.ReadAllText(paths.ConfigYamlPath)
        );
    }
    catch (Exception ex)
    {
      throw new InvalidOperationException(
        $"Failed to load configuration from {paths.ConfigYamlPath}",
        ex
      );
    }
  }
}
