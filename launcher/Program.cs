using System;
using System.Linq;
using System.Reflection;

using Autofac;
using Avalonia;
using Avalonia.Logging;
using Avalonia.ReactiveUI.Splat;
using ReactiveUI;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.ViewModel;

namespace CNC.NCO.Launcher;

internal static class Program
{
  private static readonly string[] IocTypes =
  [
    "Service",
    "View",
    "ViewModel",
    "Window"
  ];

  private static void ConfigureIoc(ContainerBuilder builder)
  {
    builder.RegisterInstance(new PathsConfig(AppContext.BaseDirectory));
    builder.RegisterType<LauncherConfigLoader>();
    builder.RegisterType<MainViewModel>().As<IScreen>();

    builder.RegisterAssemblyTypes(typeof(Program).Assembly)
      .Where(t =>
      {
        var addType = t is { IsClass: true, IsAbstract: false, IsInterface: false } &&
                      IocTypes.Any(it => t.Name.EndsWith(it));

        if (addType)
        {
          Console.WriteLine($"Adding type: {t.FullName}");
        }

        return addType;
      })
      .AsSelf();
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
      .UseReactiveUIWithAutofac(ConfigureIoc);
}
