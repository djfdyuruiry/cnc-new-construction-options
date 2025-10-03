using ReactiveUI;

namespace CNC.NCO.Launcher.ViewModel.Screens;

public class LaunchGameViewModel(IScreen hostScreen) : ScreenViewModelBase("play-game", hostScreen)
{
  public string Greeting { get; } = "Launching C&C 95....";
}
