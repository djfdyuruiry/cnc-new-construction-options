using System.Reactive;

using Avalonia.Media;

using CNC.NCO.Launcher.Service;

using ReactiveUI;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

public class StartViewModel : ScreenViewModelBase
{
  private IBrush? _backgroundImage;
  
  public IBrush? BackgroundImage
  {
    get => _backgroundImage;
    set => this.RaiseAndSetIfChanged(ref _backgroundImage, value);
  }

  public ReactiveCommand<Unit, IRoutableViewModel> Begin { get; }
  
  public StartViewModel(GameDataService gameDataService, IScreen hostScreen)
    : base("start-installer", hostScreen)
  {
    Begin = ReactiveCommand.CreateFromObservable(() => 
      HostScreen.Router.NavigateTo<SelectGameDataViewModel>()
    );
    // Install = ReactiveCommand.CreateFromTask(() => 
    //   gameDataService.Download(b =>
    //     BackgroundImage = new ImageBrush(b)
    //   )
    // );
  }
}
