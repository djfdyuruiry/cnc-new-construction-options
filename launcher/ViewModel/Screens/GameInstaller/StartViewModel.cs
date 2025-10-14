using System;
using System.Reactive;

using ReactiveUI;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

public class StartViewModel : ScreenViewModelBase
{
  public ReactiveCommand<Unit, IRoutableViewModel> Next { get; }

  public StartViewModel(IScreen hostScreen)
    : base("start-installer", hostScreen)
  {
    Next = ReactiveCommand.CreateFromObservable(() =>
      // custom install path not supported on macOS
      OperatingSystem.IsMacOS()
        ? HostScreen.Router.NavigateTo<SelectGameDataViewModel>()
        : HostScreen.Router.NavigateTo<SelectInstallPathViewModel>()
    );
  }
}
