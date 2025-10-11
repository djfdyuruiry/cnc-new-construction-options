using System;
using System.Reactive;

using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using ReactiveUI;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Model;
using CNC.NCO.Launcher.ViewModel.Screens;
using CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

namespace CNC.NCO.Launcher.ViewModel;

public class MainViewModel : ReactiveObject, IScreen
{
  private readonly LauncherConfigService _configService;
  private NewConstructionOptions? _nco;
  private bool _installerEnabled;

  public RoutingState Router { get; } = new();

  public NewConstructionOptions? Nco
  {
    get => _nco;
    private set => this.RaiseAndSetIfChanged(ref _nco, value);
  }

  public bool InstallerEnabled
  {
    get => _installerEnabled;
    private set => this.RaiseAndSetIfChanged(ref _installerEnabled, value);
  }

  public ReactiveCommand<Unit, IRoutableViewModel> Install { get; }

  public ReactiveCommand<Unit, IRoutableViewModel> Play { get; }

  public ReactiveCommand<Unit, Unit> Exit { get; }

  public MainViewModel(LauncherConfigService configService)
  {
    _configService = configService;

    Install = ReactiveCommand.CreateFromObservable(() =>
      Router.NavigateTo<StartViewModel>()
    );

    Play = ReactiveCommand.CreateFromObservable(() =>
      Router.NavigateTo<LaunchGameViewModel>()
    );

    Exit = ReactiveCommand.Create(() => 
      (Application.Current?.ApplicationLifetime as IClassicDesktopStyleApplicationLifetime)?.Shutdown()
    );
  }

  public IObservable<IRoutableViewModel> NavigateToFirstScreen()
  {
    try
    {
      Nco = _configService.Config.Nco;
      InstallerEnabled = true;

      return Nco.Installed
        ? Router.NavigateTo<LaunchGameViewModel>()
        : Router.NavigateTo<StartViewModel>();
    }
    catch (Exception ex)
    {
      return Router.NavigateTo<LauncherErrorViewModel>(vm => vm.Error = ex);
    }
  }
}
