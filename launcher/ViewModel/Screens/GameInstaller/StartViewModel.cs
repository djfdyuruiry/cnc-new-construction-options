using System.Reactive;

using ReactiveUI;

using CNC.NCO.Launcher.Service;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

public class StartViewModel : ScreenViewModelBase
{
  public ReactiveCommand<Unit, IRoutableViewModel> Begin { get; }
  
  public StartViewModel(GameDataService gameDataService, IScreen hostScreen)
    : base("start-installer", hostScreen)
  {
    Begin = ReactiveCommand.CreateFromObservable(() => 
      HostScreen.Router.NavigateTo<SelectGameDataViewModel>()
    );
  }
}
