using System;
using System.IO;
using System.Threading.Tasks;

using Autofac;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Service;
using CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;
using PowerArgs;

namespace CNC.NCO.Launcher;

internal class SilentInstaller
{
  private readonly SilentInstallerConfig _config;
  private readonly IContainer _container;

  public static async Task Run(SilentInstallerConfig config)
  {
    await (new SilentInstaller(config).Install());
  }

  public SilentInstaller(SilentInstallerConfig config)
  {
    _config = config;
    var containerBuilder = new ContainerBuilder();

    containerBuilder.RegisterAssemblyModules(typeof(Program).Assembly);

    _container = containerBuilder.Build();
  }

  private async Task RunInstallViewModel(ILifetimeScope scope)
  {
    try
    {
      var installVisitor = new SilentInstallDownloadVisitor();
      var installViewModel = scope.Resolve<InstallGameViewModel>();

      installVisitor.SetInstallLogCallback(s => installViewModel.InstallLog += s);

      installViewModel.Initialise();

      await installViewModel.InstallWith(installVisitor, scope.Resolve<Bin2IsoService>);

      if (installVisitor.HasErrored)
      { 
        await ErrorAndExit($"Installation failed due to an error: {installVisitor.Error ?? "unknown error"}");
      }
    }
    catch (Exception e)
    {
      await ErrorAndExit($"Installation failed due to an error: {e.Message}");
    }
  }

  private void ConfigureInstallation(ILifetimeScope scope)
  {
    if (_config.InstallAll)
    {
      _config.InstallTd = _config.InstallRa = _config.InstallRaTheLostFiles = true;
    }
  
    var paths = scope.Resolve<PathsConfig>();
    var launcherConfig = scope.Resolve<LauncherConfigService>().Config;

    var ncoConfig = launcherConfig.Nco;

    // install in cli path OR existing install path OR default path
    ncoConfig.PendingInstallPath = !string.IsNullOrWhiteSpace(_config.InstallPath)
      ? _config.InstallPath
      : !string.IsNullOrWhiteSpace(ncoConfig.InstallPath)
        ? ncoConfig.InstallPath
        : paths.NcoAppDataPath;

    launcherConfig.TiberianDawn.Enabled = _config.InstallTd;
    launcherConfig.RedAlert.Enabled = _config.InstallRa;
    launcherConfig.RedAlert.ZipUrlSpecs.ForEach(s => s.Enabled = _config.InstallRaTheLostFiles);
  }

  private async Task ErrorAndExit(string error)
  {
    await Console.Error.WriteLineAsync(error);
    Environment.Exit(1);
  }

  private async Task ValidateInstallConfig()
  {
    if (_config is { InstallAll: false, InstallTd: false, InstallRa: false, InstallRaTheLostFiles: false })
    {
      await ErrorAndExit("You must specify at least one install flag, see --help");
    }

    if (_config is { InstallRa: false, InstallRaTheLostFiles: true })
    {
      await ErrorAndExit(
        "You can only install The Lost Files patch (--install-ra-tlf) if you also install Red Alert (--install-ra)"
      );
    }

    if (_config.InstallPath is not null)
    {
      try
      {
        Path.GetFullPath(_config.InstallPath);
      }
      catch (Exception e)
      {
        await ErrorAndExit($"--install-path value is not a valid file system path ({e.Message})");
      }
    }
  }

  private async Task Install()
  {
    await ValidateInstallConfig();

    await using var scope = _container.BeginLifetimeScope();

    ConfigureInstallation(scope);
    await RunInstallViewModel(scope);
  }
}
