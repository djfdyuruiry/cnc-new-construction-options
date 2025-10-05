using System;
using System.Reactive.Disposables;
using Avalonia.Controls;
using Avalonia.ReactiveUI;

using CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;
using DynamicData.Binding;
using ReactiveUI;

namespace CNC.NCO.Launcher.View.Screens.GameInstaller;

public partial class InstallGameView : ReactiveUserControl<InstallGameViewModel>
{
  public InstallGameView()
  {
    InitializeComponent();

    this.WhenActivated(d =>
      ViewModel?.WhenPropertyChanged(vm => vm.InstallLog)
        .Subscribe(p => 
          this.GetControl<ScrollViewer>("InstallLogViewer").ScrollToEnd()
        )
        .DisposeWith(d)
    );
  }
}
