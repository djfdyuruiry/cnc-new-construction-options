using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive;

using Avalonia.Media;
using ReactiveUI;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Model;
using CNC.NCO.Launcher.Model.ViewModel;
using CNC.NCO.Launcher.Service;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

public class InstallGameViewModel : ScreenViewModelBase
{
  private IBrush? _backgroundImage;
  private string _installLog;

  private bool _installNotClicked;
  private bool _isInstalling;
  private bool _hasErrored;
  private bool _installFinished;

  private ObservableCollection<ItemToBeInstalled<DiscImageSource>> _enabledDiscImages;
  private ObservableCollection<ItemToBeInstalled<ZipUrlSpec>> _modsAndAddons;
  private ItemToBeInstalled<NewConstructionOptions> _nco;

  public IBrush? BackgroundImage
  {
    get => _backgroundImage;
    set => this.RaiseAndSetIfChanged(ref _backgroundImage, value);
  }

  public string InstallLog
  {
    get => _installLog;
    set => this.RaiseAndSetIfChanged(ref _installLog, value);
  }

  public bool InstallNotClicked
  {
    get => _installNotClicked;
    set => this.RaiseAndSetIfChanged(ref _installNotClicked, value);
  }

  public bool IsInstalling
  {
    get => _isInstalling;
    set => this.RaiseAndSetIfChanged(ref _isInstalling, value);
  }

  public bool HasErrored
  {
    get => _hasErrored;
    set => this.RaiseAndSetIfChanged(ref _hasErrored, value);
  }

  public bool InstallFinished
  {
    get => _installFinished;
    set => this.RaiseAndSetIfChanged(ref _installFinished, value);
  }

  public ObservableCollection<ItemToBeInstalled<DiscImageSource>> DiscImages
  {
    get => _enabledDiscImages;
    set => this.RaiseAndSetIfChanged(ref _enabledDiscImages, value);
  }

  public ObservableCollection<ItemToBeInstalled<ZipUrlSpec>> ModsAndAddons
  {
    get => _modsAndAddons;
    set => this.RaiseAndSetIfChanged(ref _modsAndAddons, value);
  }
  
  public bool ModsAndAddonsPresent => ModsAndAddons.Count > 0;

  public ItemToBeInstalled<NewConstructionOptions> Nco
  {
    get => _nco;
    set => this.RaiseAndSetIfChanged(ref _nco, value);
  }

  public ReactiveCommand<Unit, Unit> Start { get; }
  public ReactiveCommand<Unit, IRoutableViewModel> Finish { get; }

  public InstallGameViewModel(
    IScreen hostScreen,
    LauncherConfigService configService,
    GameDataService gameDataService,
    NcoReleaseService releaseService
  )
    : base("install-games", hostScreen)
  {
    InstallNotClicked = true;
    IsInstalling = false;
    InstallFinished = false;
    HasErrored = false;

    _enabledDiscImages = new ObservableCollection<ItemToBeInstalled<DiscImageSource>>(
      configService.Config
        .Games
        .SelectMany(
          // TODO: Allow user to select source and pass into this method to filter (instead of first)
          g => g.DiscImagesBySource
            .First()
            .Value
            .Where(d => d.Game.Enabled && d.Enabled)
        )
        .Select(d => new ItemToBeInstalled<DiscImageSource>(d))
        .OrderBy(d => d.Item.Game.SortOrder)
        .ThenBy(d => d.Item.SortOrder)
    );
    _modsAndAddons = new ObservableCollection<ItemToBeInstalled<ZipUrlSpec>>(
      configService.Config
        .Games
        .SelectMany(g => (g.ZipUrls ?? []).Where(z => z.Game.Enabled && z.Enabled))
        .Select(z => new ItemToBeInstalled<ZipUrlSpec>(z))
        .OrderBy(z => z.Item.Game.SortOrder)
        .ThenBy(z => z.Item.SortOrder)
    );
    _nco = new ItemToBeInstalled<NewConstructionOptions>(configService.Config.NCO);

    Start = ReactiveCommand.CreateFromTask(async () =>
    {
      InstallNotClicked = false;
      IsInstalling = false;
      InstallFinished = false;
      HasErrored = false;

      var downloadEventVisitor = new InstallDownloadEventVisitor(this);

      await gameDataService.Download(
        downloadEventVisitor,
        b => BackgroundImage = new ImageBrush(b)
      );
      await releaseService.Download(downloadEventVisitor);

      IsInstalling = false;
      InstallFinished = !HasErrored;
    });

    Finish = ReactiveCommand.CreateFromObservable(() =>
      HostScreen.Router.NavigateTo<LaunchGameViewModel>()
    );
  }
}
