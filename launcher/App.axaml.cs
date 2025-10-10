using System;

using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;
using Splat;

using CNC.NCO.Launcher.Config;
using Splat.Autofac;

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
    SaveUserConfig();
    
    try
    {
      if (Locator.GetLocator() is not AutofacDependencyResolver autofac)
      {
        Console.Error.WriteLine("WARN Unable to access Autofac dependency resolver, services will not be disposed");
        return;
      }
      
      autofac.Dispose();
    }
    catch (Exception ex)
    {
      Console.Error.WriteLine(ex);
    }
  }

  private void SaveUserConfig()
  {
    try
    {
      Locator.Current.GetService<LauncherConfigService>()?.SaveUserConfig();
    }
    catch (Exception ex)
    {
      Console.Error.WriteLine(ex);
    }
  }
}
