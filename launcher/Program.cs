using System;

using Autofac;
using Avalonia;
using Avalonia.ReactiveUI.Splat;
using ReactiveUI;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Service;
using CNC.NCO.Launcher.View.Screens;
using CNC.NCO.Launcher.ViewModel;
using CNC.NCO.Launcher.ViewModel.Screens;

namespace CNC.NCO.Launcher;

internal static class Program
{
  private static readonly string[] IocTypes =
  [
    "Service"
  ];

  private static void ConfigureIocContainer(ContainerBuilder builder)
  {
    builder.RegisterInstance(new PathsConfig(AppContext.BaseDirectory));
    builder.RegisterType<LauncherConfigLoader>();

    builder.RegisterModule<ServicesModule>();
    builder.RegisterModule<ScreensModule>();
    builder.RegisterModule<ScreenViewModelsModule>();

    builder.RegisterType<MainViewModel>().SingleInstance().AsSelf().As<IScreen>();
    builder.RegisterType<MainWindow>().SingleInstance().AsSelf();
  }

  // Initialization code. Don't use any Avalonia, third-party APIs or any
  // SynchronizationContext-reliant code before AppMain is called: things aren't initialized
  // yet and stuff might break.
  [STAThread]
  public static void Main(string[] args) => BuildAvaloniaApp()
    .StartWithClassicDesktopLifetime(args);

  // Avalonia configuration, don't remove; also used by visual designer.
  // ReSharper disable once MemberCanBePrivate.Global
  public static AppBuilder BuildAvaloniaApp()
    => AppBuilder.Configure<App>()
      .UsePlatformDetect()
      .WithInterFont()
      .LogToTrace()
      .UseReactiveUIWithAutofac(ConfigureIocContainer);

}
