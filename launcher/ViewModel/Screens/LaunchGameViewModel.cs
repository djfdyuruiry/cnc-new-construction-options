using System;
using System.Diagnostics;
using System.IO;
using System.Reactive;
using System.Threading.Tasks;
using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Model;
using ReactiveUI;

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

  public ReactiveCommand<Unit, Unit> LaunchTd { get; }
  public ReactiveCommand<Unit, Unit> LaunchRa { get; }

  public LaunchGameViewModel(LauncherConfigService configService, IScreen hostScreen)
    : base("play-game", hostScreen)
  {
    Nco = configService.Config.NCO;
    Td = configService.Config.TiberianDawn;
    Ra = configService.Config.RedAlert;

    LaunchTd = ReactiveCommand.Create(() =>
      LaunchGame(configService.Config.TiberianDawn, () => TdProcess, p => TdProcess = p)
    );

    LaunchRa = ReactiveCommand.Create(() => 
      LaunchGame(configService.Config.TiberianDawn, () => RaProcess, p => RaProcess = p)
    );
  }

  private void LaunchGame(GameDataConfig game, Func<Process?> getProcess, Action<Process?> setProcess)
  {
    LaunchFailed = false;

    var existingProcess = getProcess();

    if (existingProcess is not null && !existingProcess.HasExited)
    {
      Console.Error.WriteLine($"WARN: Game '{game.DisplayName}' already running");
      return;
    }

    // TODO: Review macos path when NcoReleaseService deploys .app to Applications
    var binaryName = OperatingSystem.IsWindows()
      ? $"${game.Binary}.exe"
      : OperatingSystem.IsMacOS() ? $"/Applications/${game.Binary}.app/Contents/MacOS/${game.Binary}" : game.Binary;

    var gameProcessInfo = new ProcessStartInfo
    {
      FileName = Path.Join(Nco.InstallPath, game.InstallPostfix, binaryName),
      WorkingDirectory = Path.Join(Nco.InstallPath, game.InstallPostfix),
      RedirectStandardOutput = true,
      RedirectStandardError = true
    };

    var gameProcess = Process.Start(gameProcessInfo);

    setProcess(gameProcess);

    if (gameProcess is null || gameProcess.HasExited)
    {
      LaunchFailed = true;
    }
  }
}
