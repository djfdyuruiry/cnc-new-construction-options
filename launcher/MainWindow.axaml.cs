using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Media;

namespace CNC.NCO.Launcher;

public partial class MainWindow : Window
{
  private readonly GameDataService _gameDataService;

  public MainWindow()
  {
    _gameDataService = new GameDataService();
  
    InitializeComponent();
  }

  protected override void OnLoaded(RoutedEventArgs e)
  {
    base.OnLoaded(e);
    
    _ = _gameDataService.LoadData(b => Background = new ImageBrush(b));
  }
}