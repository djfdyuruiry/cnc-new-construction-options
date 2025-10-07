using System.Reactive;
using CNC.NCO.Launcher.Config;
using ReactiveUI;

using CNC.NCO.Launcher.Service;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

public class StartViewModel : ScreenViewModelBase
{
  public ReactiveCommand<Unit, IRoutableViewModel> Next { get; }
  
  public StartViewModel(IScreen hostScreen)
    : base("start-installer", hostScreen)
  {
    Next = ReactiveCommand.CreateFromObservable(() => 
      HostScreen.Router.NavigateTo<SelectInstallPathViewModel>()
    );
  }
}
