using System.Reactive;
using CNC.NCO.Launcher.Config;
using ReactiveUI;

using CNC.NCO.Launcher.Service;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

public class StartViewModel : ScreenViewModelBase
{
  public bool IsInstalled { get; }
  public ReactiveCommand<Unit, IRoutableViewModel> Next { get; }
  
  public StartViewModel(LauncherConfigService configService, IScreen hostScreen)
    : base("start-installer", hostScreen)
  {
    IsInstalled = configService.Config.NCO.Installed;

    Next = ReactiveCommand.CreateFromObservable(() => 
      HostScreen.Router.NavigateTo<SelectInstallPathViewModel>()
    );
  }
}
