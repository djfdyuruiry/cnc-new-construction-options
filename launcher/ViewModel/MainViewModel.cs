using System;
using System.Reactive;

using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using ReactiveUI;

using CNC.NCO.Launcher.ViewModel.Screens;
using CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

namespace CNC.NCO.Launcher.ViewModel;

public class MainViewModel : ReactiveObject, IScreen
{
  public RoutingState Router { get; } = new();

  public ReactiveCommand<Unit, IRoutableViewModel> Install { get; }

  public ReactiveCommand<Unit, IRoutableViewModel> Play { get; }

  public ReactiveCommand<Unit, Unit> Exit { get; }

  public MainViewModel()
  {
    Install = ReactiveCommand.CreateFromObservable(() =>
      Router.NavigateTo<StartViewModel>()
    );

    Play = ReactiveCommand.CreateFromObservable(() =>
      Router.NavigateTo<LaunchGameViewModel>()
    );

    Exit = ReactiveCommand.Create(() =>
    {
      if (Application.Current?.ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
      {
        desktop.Shutdown();
      }
      else
      {
        Environment.Exit(0);
      }
    });
  }
}
