using System.Reactive;
using CNC.NCO.Launcher.ViewModel.Screens;
using ReactiveUI;

using CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

namespace CNC.NCO.Launcher.ViewModel;

public class MainViewModel : ReactiveObject, IScreen
{
  public RoutingState Router { get; } = new();

  public ReactiveCommand<Unit, IRoutableViewModel> Play { get; }

  public ReactiveCommand<Unit, IRoutableViewModel> Install { get; }

  public MainViewModel()
  {
    Play = ReactiveCommand.CreateFromObservable(() => 
      Router.Execute<LaunchGameViewModel>()
    );

    Install = ReactiveCommand.CreateFromObservable(() => 
      Router.Execute<StartViewModel>()
    );
  }
}
