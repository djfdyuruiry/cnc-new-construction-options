using Avalonia.ReactiveUI;

using CNC.NCO.Launcher.ViewModel;
using Splat;

namespace CNC.NCO.Launcher;

public partial class MainWindow : ReactiveWindow<MainViewModel>
{
  public MainWindow()
  {
    InitializeComponent();

    ViewModel = Locator.Current.GetService<MainViewModel>();
  }
}
