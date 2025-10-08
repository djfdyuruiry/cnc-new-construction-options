using System.Reactive;

using ReactiveUI;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Model;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

public class SelectGameDataViewModel : ScreenViewModelBase
{
  private LauncherConfig _config;

  public LauncherConfig Config
  {
    get => _config;
    set => this.RaiseAndSetIfChanged(ref _config, value);
  }

  public ReactiveCommand<Unit, IRoutableViewModel> Back { get; }

  public ReactiveCommand<Unit, IRoutableViewModel> Next { get; }

  public SelectGameDataViewModel(LauncherConfigService configService, IScreen hostScreen)
    : base("select-games", hostScreen)
  {
    Back = ReactiveCommand.CreateFromObservable(() => 
      HostScreen.Router.NavigateTo<SelectInstallPathViewModel>()
    );
    Next = ReactiveCommand.CreateFromObservable(() => 
      HostScreen.Router.NavigateTo<InstallGameViewModel>()
    );

    this.SafeWhenNavigatedTo(() =>
      Config = configService.Config
    );
  }
}
