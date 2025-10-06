using System;
using System.Reactive;

using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Model;
using ReactiveUI;

using CNC.NCO.Launcher.ViewModel.Screens;
using CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

namespace CNC.NCO.Launcher.ViewModel;

public class MainViewModel : ReactiveObject, IScreen
{
  private readonly LauncherConfig _config;

  public RoutingState Router { get; } = new();

  public NewConstructionOptions NcoConfig => _config.NCO;

  public ReactiveCommand<Unit, IRoutableViewModel> Install { get; }

  public ReactiveCommand<Unit, IRoutableViewModel> Play { get; }

  public ReactiveCommand<Unit, Unit> Exit { get; }

  public MainViewModel(LauncherConfigService configService)
  {
    _config = configService.Config;

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

  public IObservable<IRoutableViewModel> NavigateToFirstScreen() =>
    NcoConfig.Installed ? Router.NavigateTo<LaunchGameViewModel>() : Router.NavigateTo<StartViewModel>();
}
