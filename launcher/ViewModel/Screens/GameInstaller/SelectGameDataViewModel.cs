using System;
using System.Collections.ObjectModel;
using System.Reactive;

using CNC.NCO.Launcher.Config;
using ReactiveUI;

using CNC.NCO.Launcher.Model;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

public class SelectGameDataViewModel : ScreenViewModelBase
{
  public LauncherConfig Config { get; }

  public ReactiveCommand<Unit, IRoutableViewModel> Back { get; }

  public ReactiveCommand<Unit, IRoutableViewModel> Next { get; }

  public SelectGameDataViewModel(LauncherConfigService configService, IScreen hostScreen)
    : base("select-games", hostScreen)
  {
    Config = configService.Config;

    throw new Exception("Oh shit!");
    
    Back = ReactiveCommand.CreateFromObservable(() => 
      HostScreen.Router.NavigateTo<SelectInstallPathViewModel>()
    );
    Next = ReactiveCommand.CreateFromObservable(() => 
      HostScreen.Router.NavigateTo<InstallGameViewModel>()
    );
  }
}
