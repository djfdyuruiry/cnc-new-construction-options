using Avalonia.Controls;
using Avalonia.ReactiveUI;
using ReactiveUI;
using Splat;

using CNC.NCO.Launcher.ViewModel;

namespace CNC.NCO.Launcher;

public partial class MainWindow : ReactiveWindow<MainViewModel>
{
  public MainWindow()
  {
    InitializeComponent();

    if (!Design.IsDesignMode)
    {
      ViewModel = Locator.Current.GetService<MainViewModel>();
    }

    this.WhenActivated(_ =>
      ViewModel?.NavigateToFirstScreen()
    );
  }
}
