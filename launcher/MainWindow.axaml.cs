using System;
using System.IO;

using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Media;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Service;

namespace CNC.NCO.Launcher;

public partial class MainWindow : Window
{
  private readonly GameDataService _gameDataService;

  public MainWindow()
  {
    _gameDataService = new GameDataService(
      new LauncherConfigLoader(Path.Join(AppContext.BaseDirectory, "config.yml")),
      new Bin2IsoService(Path.Join(AppContext.BaseDirectory, "tools")),
      Path.Join(AppContext.BaseDirectory, ".cache")
    );

    InitializeComponent();
  }

  protected override void OnLoaded(RoutedEventArgs e)
  {
    base.OnLoaded(e);

    _ = _gameDataService.Download(b => Background = new ImageBrush(b));
  }
}
