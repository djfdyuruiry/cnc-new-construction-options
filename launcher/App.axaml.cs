using System;

using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;
using Splat;

using CNC.NCO.Launcher.Config;

namespace CNC.NCO.Launcher;

public partial class App : Application 
{
  public override void Initialize()
  {
    AvaloniaXamlLoader.Load(this);
  }

  public override void OnFrameworkInitializationCompleted()
  {
    if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
    {
      desktop.Exit += OnExit;
      desktop.MainWindow = new MainWindow();
    }

    base.OnFrameworkInitializationCompleted();
  }

  private void OnExit(object? sender, ControlledApplicationLifetimeExitEventArgs e)
  {
    try
    {
      SaveUserConfig();
    }
    catch (Exception ex)
    {
      Console.Error.WriteLine(ex);
    }
  }

  private void SaveUserConfig() => Locator.Current.GetService<LauncherConfigService>()?.SaveUserConfig();
}
