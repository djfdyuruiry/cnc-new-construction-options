using System;
using System.Reactive;
using Autofac;
using ReactiveUI;

using CNC.NCO.Launcher.ViewModel;
using CNC.NCO.Launcher.ViewModel.Screens;
using Splat;

namespace CNC.NCO.Launcher;

public class AppModule : Module
{
  protected override void Load(ContainerBuilder builder)
  {
    // singletons for root mvvm container
    builder.RegisterType<MainViewModel>()
      .SingleInstance()
      .AsSelf()
      .As<IScreen>();

    builder.RegisterType<MainWindow>()
      .SingleInstance()
      .AsSelf();

    // global exception handler
    RxApp.DefaultExceptionHandler = Observer.Create<Exception>(e =>
      Locator.Current
        .GetService<MainViewModel>()?
        .Router
        .NavigateTo<LauncherErrorViewModel>(vm => vm.Error = e)
    );
  }
}
