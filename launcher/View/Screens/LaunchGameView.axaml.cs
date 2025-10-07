using System;
using System.Reactive.Disposables;
using System.Reactive.Linq;
using Avalonia.Controls;
using Avalonia.ReactiveUI;
using ReactiveUI;

using CNC.NCO.Launcher.ViewModel.Screens;

namespace CNC.NCO.Launcher.View.Screens;

public partial class LaunchGameView : ReactiveUserControl<LaunchGameViewModel>
{
  public LaunchGameView()
  {
    InitializeComponent();

    this.WhenActivated(d =>
    {
      if (ViewModel is null)
      {
        return;
      }

      Observable.FromEventPattern(TdPanel, nameof(TdPanel.PointerReleased))
        .InvokeCommand(ViewModel.LaunchTd)
        .DisposeWith(d);

      Observable.FromEventPattern(RaPanel, nameof(RaPanel.PointerReleased))
        .InvokeCommand(ViewModel.LaunchRa)
        .DisposeWith(d);
    });
  }
}
