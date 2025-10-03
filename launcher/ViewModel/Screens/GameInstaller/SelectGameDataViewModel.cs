using CNC.NCO.Launcher.Config;
using ReactiveUI;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

public class SelectGameDataViewModel : ScreenViewModelBase
{
  public SelectGameDataViewModel(LauncherConfigLoader configLoader, IScreen hostScreen)
    : base("select-games", hostScreen)
  {
    this.(() =>
    {

    });
  }
}
