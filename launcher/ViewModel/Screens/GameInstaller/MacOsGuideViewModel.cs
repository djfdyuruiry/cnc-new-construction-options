using System.Reactive;

using ReactiveUI;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

public class MacOsGuideViewModel : ScreenViewModelBase
{
  public ReactiveCommand<Unit, IRoutableViewModel> Done { get; }

  public MacOsGuideViewModel(string urlPathSegment, IScreen hostScreen) : base(urlPathSegment, hostScreen)
  {
    Done = ReactiveCommand.CreateFromObservable(() =>
      HostScreen.Router.NavigateTo<LaunchGameViewModel>()
    );
  }
}
