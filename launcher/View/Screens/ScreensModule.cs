using Autofac;
using Avalonia.ReactiveUI;

namespace CNC.NCO.Launcher.View.Screens;

public class ScreensModule : Module
{
  protected override void Load(ContainerBuilder builder)
  {
    builder.RegisterAssemblyTypes(typeof(ScreensModule).Assembly)
      .Where(t =>
        (t.Namespace?.StartsWith("CNC.NCO.Launcher.View.Screens") ?? false) &&
        t is { IsClass: true, IsAbstract: false, BaseType: not null, BaseType.IsGenericType: true } &&
        t.BaseType!.GetGenericTypeDefinition() == typeof(ReactiveUserControl<>)
      )
      .AsSelf();
  }
}
