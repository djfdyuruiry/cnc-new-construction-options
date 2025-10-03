using Autofac;

namespace CNC.NCO.Launcher.ViewModel.Screens;

public class ScreenViewModelsModule : Module
{
  protected override void Load(ContainerBuilder builder)
  {
    builder.RegisterAssemblyTypes(typeof(ScreenViewModelsModule).Assembly)
      .Where(t =>
        (t.Namespace?.StartsWith("CNC.NCO.Launcher.ViewModel.Screens") ?? false) &&
        t is { IsClass: true, IsAbstract: false, BaseType: not null } &&
        t.BaseType == typeof(ScreenViewModelBase)
      )
      .AsSelf();
  }
}
