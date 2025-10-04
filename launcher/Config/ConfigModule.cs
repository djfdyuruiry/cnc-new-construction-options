using System;

using Autofac;

namespace CNC.NCO.Launcher.Config;

public class ConfigModule : Module
{
  protected override void Load(ContainerBuilder builder)
  {
    var pathsConfig = new PathsConfig(
      AppContext.BaseDirectory, 
      Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData)
    );
    var configService = new LauncherConfigService(pathsConfig);

    builder.RegisterInstance(pathsConfig).SingleInstance().AsSelf();
    builder.RegisterInstance(configService).SingleInstance().AsSelf();
  }
}
