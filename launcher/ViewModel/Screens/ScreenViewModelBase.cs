using ReactiveUI;

namespace CNC.NCO.Launcher.ViewModel.Screens;

public abstract class ScreenViewModelBase(string urlPathSegment, IScreen hostScreen)
  : ReactiveObject,
    IRoutableViewModel
{
  public string? UrlPathSegment { get; } = urlPathSegment;
  public IScreen HostScreen { get; } = hostScreen;
}
