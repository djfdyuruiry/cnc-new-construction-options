using System;

using ReactiveUI;

namespace CNC.NCO.Launcher.ViewModel.Screens;

public class LauncherErrorViewModel(IScreen hostScreen) : ScreenViewModelBase("error", hostScreen)
{
  public Exception? Error { get; set; }
}
