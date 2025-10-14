using System.Reactive;

using ReactiveUI;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

public class MacOsGuideViewModel : ScreenViewModelBase
{
  public ReactiveCommand<Unit, IRoutableViewModel> Next { get; }

  public MacOsGuideViewModel(string urlPathSegment, IScreen hostScreen) : base(urlPathSegment, hostScreen)
  {
    Next = ReactiveCommand.CreateFromObservable(() =>
      HostScreen.Router.NavigateTo<LaunchGameViewModel>()
    );
  }
}
