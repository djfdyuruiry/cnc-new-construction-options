using System;
using System.Runtime.Versioning;
using System.Threading.Tasks;

using Autofac;
using Avalonia;
using Avalonia.ReactiveUI.Splat;
using PowerArgs;

[assembly: SupportedOSPlatform("windows")]
[assembly: SupportedOSPlatform("linux")]
[assembly: SupportedOSPlatform("osx")]

namespace CNC.NCO.Launcher;

internal static class Program
{
  [STAThread]
  public static async Task Main(string[] args)
  {
    SilentInstallerConfig? installerConfig = null;

    try
    {
      installerConfig = Args.Parse<SilentInstallerConfig>(args);
    }
    catch (ArgException e)
    {
      await Console.Error.WriteLineAsync(e.Message);
      Console.Error.WriteLine(ArgUsage.GenerateUsageFromTemplate<SilentInstallerConfig>());
      Environment.Exit(1);
    }

    if (installerConfig?.SilentInstall ?? false)
    {
      await SilentInstaller.Run(installerConfig);
    }
    else
    {
      BuildAvaloniaApp().StartWithClassicDesktopLifetime(args);
    }
  }

  private static AppBuilder BuildAvaloniaApp() =>
    AppBuilder.Configure<App>()
      .UsePlatformDetect()
      .WithInterFont()
      .LogToTrace()
      .UseReactiveUIWithAutofac(c =>
        // module auto-discovery
        c.RegisterAssemblyModules(typeof(Program).Assembly)
      );
}
