using System;
using System.Reactive.Disposables;
using System.Reactive.Linq;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
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
      BackgroundShaderGrid.Background = new SolidColorBrush(
        Equals("Dark", Application.Current?.ActualThemeVariant.Key)
          ? Colors.Black
          : Colors.White
      );

      if (ViewModel is null)
      {
        return;
      }

      ViewModel.WhenValueChanged(x => x.InstallLog)
        .ObserveOn(RxApp.MainThreadScheduler)
        .Subscribe(_ =>
          this.GetControl<ScrollViewer>("InstallLogViewer").ScrollToEnd()
        )
        .DisposeWith(d);
    });
  }
}
