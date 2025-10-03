using System;

using Autofac;

namespace CNC.NCO.Launcher.Config;

public class ConfigModule : Module
{
  protected override void Load(ContainerBuilder builder)
  {
    var pathsConfig = new PathsConfig(AppContext.BaseDirectory);
    var configLoader = new LauncherConfigLoader(pathsConfig);

    builder.RegisterInstance(pathsConfig).SingleInstance().AsSelf();
    builder.RegisterInstance(configLoader).SingleInstance().AsSelf();
    builder.RegisterInstance(() => configLoader.Load()).SingleInstance().AsSelf();
  }
}
