using System;
using System.Diagnostics;
using System.IO;
using System.Reactive;

using ReactiveUI;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Model;
using CNC.NCO.Launcher.Util;

namespace CNC.NCO.Launcher.ViewModel.Screens;

public class LaunchGameViewModel : ScreenViewModelBase
{
  private readonly PathsConfig _pathsConfig;
  private Process? _tdProcess;
  private Process? _raProcess;
  private bool _launchFailed;
  private LauncherConfig? _config;

  public LauncherConfig? Config
  {
    get => _config;
    set => this.RaiseAndSetIfChanged(ref _config, value);
  }

  public Process? TdProcess
  {
    get => _tdProcess;
    set => this.RaiseAndSetIfChanged(ref _tdProcess, value);
  }

  public Process? RaProcess
  {
    get => _raProcess;
    set => this.RaiseAndSetIfChanged(ref _raProcess, value);
  }
  
  public bool LaunchFailed
  {
    get => _launchFailed;
    set => this.RaiseAndSetIfChanged(ref _launchFailed, value);
  }

  public ReactiveCommand<EventPattern<object>, Unit> LaunchTd { get; }
  public ReactiveCommand<EventPattern<object>, Unit> LaunchRa { get; }

  public LaunchGameViewModel(LauncherConfigService configService, PathsConfig pathsConfig, IScreen hostScreen)
    : base("play-game", hostScreen)
  {
    _pathsConfig = pathsConfig;
    LaunchFailed = false;

    LaunchTd = ReactiveCommand.Create((EventPattern<object> _) =>
      LaunchGame(Config!.TiberianDawn, () => TdProcess, p => TdProcess = p)
    );

    LaunchRa = ReactiveCommand.Create((EventPattern<object> _)  => 
      LaunchGame(Config!.RedAlert, () => RaProcess, p => RaProcess = p)
    );

    SafeWhenNavigatedTo(() =>
      Config = configService.Config
    );
  }

  private void LaunchGame(GameDataConfig game, Func<Process?> getProcess, Action<Process?> setProcess)
  {
    var existingProcess = getProcess();

    if (existingProcess is not null && !existingProcess.HasExited)
    {
      Console.Error.WriteLine($"WARN: Game '{game.DisplayName}' already running");
      return;
    }
    
    // TODO: Review macos path when NcoReleaseService deploys .app to Applications
    LaunchFailed = false;

    var stdOutLog = Path.Join(_pathsConfig.NcoAppDataPath, $"{game.InstallPostfix}-stdout.log");
    var stdErrLog = Path.Join(_pathsConfig.NcoAppDataPath, $"{game.InstallPostfix}-stderr.log");

    File.WriteAllText(stdOutLog, string.Empty);
    File.WriteAllText(stdErrLog, string.Empty);

    var gameProcess = ProcessUtils.ExecWithCallback(
      new ProcessStartInfo
      {
        FileName = Path.Join(Config!.Nco.InstallPath, game.InstallPostfix, game.PlatformBinary),
        WorkingDirectory = Path.Join(Config.Nco.InstallPath, game.InstallPostfix),
        RedirectStandardOutput = true,
        RedirectStandardError = true
      },
      r =>
      {
        LaunchFailed = r.ExitCode != 0;

        File.WriteAllText(stdOutLog, r.StdOut);
        File.WriteAllText(stdErrLog, r.StdErr);

        getProcess()?.Dispose();
        setProcess(null);
      }
    );

    setProcess(gameProcess);
  }
}
