using System;
using System.IO;

using YamlDotNet.Serialization;

using CNC.NCO.Launcher.Model;

namespace CNC.NCO.Launcher.Config;

public class LauncherConfigLoader(string configPath)
{
  public LauncherConfig Load()
  {
    try
    {
      return new DeserializerBuilder()
        .Build()
        .Deserialize<LauncherConfig>(
          File.ReadAllText(configPath)
        );
    }
    catch (Exception ex)
    {
      throw new InvalidOperationException(
        $"Failed to load configuration from {configPath}",
        ex
      );
    }
  }
}
