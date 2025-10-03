using Autofac;
using ReactiveUI;

using CNC.NCO.Launcher.ViewModel;

namespace CNC.NCO.Launcher;

public class AppModule : Module
{
  protected override void Load(ContainerBuilder builder)
  {
    // singletons for root mvvm container
    builder.RegisterType<MainViewModel>()
      .SingleInstance()
      .AsSelf()
      .As<IScreen>();

    builder.RegisterType<MainWindow>()
      .SingleInstance()
      .AsSelf();
  }
}
