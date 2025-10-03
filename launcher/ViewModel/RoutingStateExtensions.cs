using System;

using ReactiveUI;
using Splat;

namespace CNC.NCO.Launcher.ViewModel;

public static class RoutingStateExtensions
{
  public static IObservable<IRoutableViewModel> NavigateTo<T>(this RoutingState router) where T : IRoutableViewModel
  {
    var viewModel = Locator.Current.GetService<T>()
                    ?? throw new InvalidOperationException($"Could not navigate toa view model: {typeof(T)}");

    return router.Navigate.Execute(viewModel);
  }
}
