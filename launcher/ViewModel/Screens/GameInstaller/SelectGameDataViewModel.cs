using System.Collections.ObjectModel;

using CNC.NCO.Launcher.Config;

using ReactiveUI;

using CNC.NCO.Launcher.Model;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

public class SelectGameDataViewModel : ScreenViewModelBase
{
  private GameDataConfig _td;
  private GameDataConfig _ra;
  private ObservableCollection<DiscImage> _tdDiscImages;
  private ObservableCollection<DiscImage> _raDiscImages;

  public GameDataConfig TiberianDawn
  {
    get => _td;
    set => this.RaiseAndSetIfChanged(ref _td, value);
  }

  public GameDataConfig RedAlert
  {
    get => _ra;
    set => this.RaiseAndSetIfChanged(ref _ra, value);
  }

  public ObservableCollection<DiscImage> TdDiscImages
  {
    get => _tdDiscImages;
    set => this.RaiseAndSetIfChanged(ref _tdDiscImages, value);
  }

  public ObservableCollection<DiscImage> RaDiscImages
  {
    get => _raDiscImages;
    set => this.RaiseAndSetIfChanged(ref _raDiscImages, value);
  }

  public SelectGameDataViewModel(LauncherConfigLoader configLoader, IScreen hostScreen)
    : base("select-games", hostScreen)
  {
    var config = configLoader.Load();

    _td = config.TiberianDawn;
    _ra = config.RedAlert;
    _tdDiscImages = new ObservableCollection<DiscImage>(config.TiberianDawn.DiscImages);
    _raDiscImages = new ObservableCollection<DiscImage>(config.RedAlert.DiscImages);
  }
}
