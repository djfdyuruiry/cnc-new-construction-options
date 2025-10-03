using System;

using Autofac;
using Avalonia;
using Avalonia.ReactiveUI.Splat;

namespace CNC.NCO.Launcher;

internal static class Program
{
  [STAThread]
  public static void Main(string[] args) =>
    AppBuilder.Configure<App>()
      .UsePlatformDetect()
      .WithInterFont()
      .LogToTrace()
      .UseReactiveUIWithAutofac(c =>
        // module auto-discovery
        c.RegisterAssemblyModules(typeof(Program).Assembly)
      )
      .StartWithClassicDesktopLifetime(args);
}
