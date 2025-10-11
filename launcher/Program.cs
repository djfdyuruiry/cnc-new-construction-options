using System;
using System.Runtime.Versioning;

using Autofac;
using Avalonia;
using Avalonia.ReactiveUI.Splat;

[assembly: SupportedOSPlatform("windows")]
[assembly: SupportedOSPlatform("linux")]
[assembly: SupportedOSPlatform("osx")]

namespace CNC.NCO.Launcher;

internal static class Program
{
  [STAThread]
  public static void Main(string[] args) => BuildAvaloniaApp().StartWithClassicDesktopLifetime(args);

  public static AppBuilder BuildAvaloniaApp() =>
    AppBuilder.Configure<App>()
      .UsePlatformDetect()
      .WithInterFont()
      .LogToTrace()
      .UseReactiveUIWithAutofac(c =>
        // module auto-discovery
        c.RegisterAssemblyModules(typeof(Program).Assembly)
      );
}
