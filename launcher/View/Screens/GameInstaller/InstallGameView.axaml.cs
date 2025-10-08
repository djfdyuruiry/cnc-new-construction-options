using System;
using System.Reactive.Disposables;
using System.Reactive.Linq;

using Avalonia.Controls;
using Avalonia.ReactiveUI;
using DynamicData.Binding;
using ReactiveUI;

using CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

namespace CNC.NCO.Launcher.View.Screens.GameInstaller;

public partial class InstallGameView : ReactiveUserControl<InstallGameViewModel>
{
  public InstallGameView()
  {
    InitializeComponent();

    this.WhenActivated(d =>
    {
      if (ViewModel is null)
      {
        return;
      }

      ViewModel.WhenPropertyChanged(vm => vm.InstallLog)
        .ObserveOn(RxApp.MainThreadScheduler)
        .Subscribe(p =>
          this.GetControl<ScrollViewer>("InstallLogViewer").ScrollToEnd()
        )
        .DisposeWith(d);
    });
  }
}
