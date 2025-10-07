using System;
using System.Diagnostics;
using System.IO;
using System.Reactive;

using ReactiveUI;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Model;

namespace CNC.NCO.Launcher.ViewModel.Screens;

public class LaunchGameViewModel : ScreenViewModelBase
{
  private Process? _tdProcess;
  private Process? _raProcess;
  private bool _launchFailed;

  public NewConstructionOptions Nco { get; }
  public GameDataConfig Td { get; }
  public GameDataConfig Ra { get; }

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

  public LaunchGameViewModel(LauncherConfigService configService, IScreen hostScreen)
    : base("play-game", hostScreen)
  {
    Nco = configService.Config.NCO;
    Td = configService.Config.TiberianDawn;
    Ra = configService.Config.RedAlert;

    LaunchTd = ReactiveCommand.Create((EventPattern<object> _) =>
      LaunchGame(configService.Config.TiberianDawn, () => TdProcess, p => TdProcess = p)
    );

    LaunchRa = ReactiveCommand.Create((EventPattern<object> _)  => 
      LaunchGame(configService.Config.RedAlert, () => RaProcess, p => RaProcess = p)
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
    var gameProcess = Process.Start(
      new ProcessStartInfo
      {
        FileName = Path.Join(Nco.InstallPath, game.InstallPostfix, game.PlatformBinary),
        WorkingDirectory = Path.Join(Nco.InstallPath, game.InstallPostfix),
        RedirectStandardOutput = true,
        RedirectStandardError = true
      }
    );

    if (gameProcess?.HasExited ?? true)
    {
      LaunchFailed = true;
      gameProcess?.Dispose();

      return;
    }

    setProcess(gameProcess);

    gameProcess.EnableRaisingEvents = true;
    gameProcess.Exited += (p, _) =>
    {
      var process = p as Process;

      LaunchFailed = (process?.ExitCode ?? -1) != 0;

      process?.Dispose();
      setProcess(null);
    };
  }
}
