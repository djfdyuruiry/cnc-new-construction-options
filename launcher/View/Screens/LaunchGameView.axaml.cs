using Avalonia.ReactiveUI;

using CNC.NCO.Launcher.ViewModel.Screens;

namespace CNC.NCO.Launcher.View.Screens;

public partial class LaunchGameView : ReactiveUserControl<LaunchGameViewModel>
{
  public LaunchGameView()
  {
    InitializeComponent();
    
    // TODO: reimplement background loading during install
    // _ = _gameDataService.Download(b => Background = new ImageBrush(b));
  }
}
