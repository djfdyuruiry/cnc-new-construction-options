using System;
using System.Reactive.Disposables;

using ReactiveUI;

namespace CNC.NCO.Launcher.ViewModel.Screens;

public abstract class ScreenViewModelBase(string urlPathSegment, IScreen hostScreen)
  : ReactiveObject,
    IRoutableViewModel
{
  public string? UrlPathSegment { get; } = urlPathSegment;
  public IScreen HostScreen { get; } = hostScreen;
  
  /**
   * WhenNavigatedTo wrapper that renders the LauncherErrorView screen if an exception is raised.
   */
  protected IDisposable SafeWhenNavigatedTo(Action onNavigatedTo)
  {
    return this.WhenNavigatedTo(() => 
    {
      try
      {
        onNavigatedTo();
      }
      catch (Exception ex)
      {
        return HostScreen.Router
          .NavigateTo<LauncherErrorViewModel>(vm => vm.Error = ex)
          .Subscribe();
      }

      return Disposable.Empty;
    });
  }

  /**
   * WhenNavigatedTo wrapper that renders the LauncherErrorView screen if an exception is raised.
   */
  protected IDisposable SafeWhenNavigatedTo(Func<IDisposable?> onNavigatedTo)
  {
    return this.WhenNavigatedTo(() => 
    {
      try
      {
        return onNavigatedTo() ?? Disposable.Empty;
      }
      catch (Exception ex)
      {
        return HostScreen.Router
          .NavigateTo<LauncherErrorViewModel>(vm => vm.Error = ex)
          .Subscribe();
      }
    });
  }
}
