using System;
using System.Reactive.Disposables;

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

    this.WhenActivated(d =>
      ReactiveCommand.CreateFromObservable(ViewModel!.NavigateToFirstScreen)
        .Execute()
        .Subscribe()
        .DisposeWith(d)
    );
  }
}
