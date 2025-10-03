using Avalonia.ReactiveUI;

using CNC.NCO.Launcher.ViewModel;

namespace CNC.NCO.Launcher;

public partial class MainWindow : ReactiveWindow<MainViewModel>
{
  public MainWindow(MainViewModel viewModel)
  {
    InitializeComponent();

    ViewModel = viewModel;
  }
}
