using System;

using Autofac;

namespace CNC.NCO.Launcher.Config;

public class ConfigModule : Module
{
  protected override void Load(ContainerBuilder builder)
  {
    var pathsConfig = new PathsConfig(
      AppContext.BaseDirectory,
      Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
      Environment.GetFolderPath(Environment.SpecialFolder.UserProfile)
    );

    builder.RegisterInstance(pathsConfig).SingleInstance().AsSelf();
    builder.RegisterType<LauncherConfigService>().SingleInstance().AsSelf();
  }
}
