using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reactive.Disposables;
using System.Reactive.Linq;
using System.Threading.Tasks;

using Avalonia.Media.Imaging;
using DynamicData.Binding;
using ReactiveUI;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Model;
using CNC.NCO.Launcher.Model.ViewModel;
using CNC.NCO.Launcher.Service;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

public class InstallGameViewModel : ScreenViewModelBase
{
  private readonly GameDataService _gameDataService;
  private readonly NcoReleaseService _releaseService;
  private readonly PathsConfig _paths;

  private bool _isInstalling;
  private string _installLog;
  private Bitmap? _currentSplashScreen;
  private bool _hasErrored;
  private bool _installFinished;
  private IList<ItemToBeInstalled<DiscImageSource>>? _discImages;
  private IList<ItemToBeInstalled<ZipUrlSpec>>? _modsAndAddons;
  private ItemToBeInstalled<NewConstructionOptions>? _nco;

  public bool IsInstalling
  {
    get => _isInstalling;
    set => this.RaiseAndSetIfChanged(ref _isInstalling, value);
  }

  public string InstallLog
  {
    get => _installLog;
    set => this.RaiseAndSetIfChanged(ref _installLog, value);
  }

  public Bitmap? CurrentSplashScreen
  {
    get => _currentSplashScreen;
    set => this.RaiseAndSetIfChanged(ref _currentSplashScreen, value);
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

  public IList<ItemToBeInstalled<DiscImageSource>>? DiscImages
  {
    get => _discImages;
    set => this.RaiseAndSetIfChanged(ref _discImages, value);
  }

  public IList<ItemToBeInstalled<ZipUrlSpec>>? ModsAndAddons
  {
    get => _modsAndAddons;
    set => this.RaiseAndSetIfChanged(ref _modsAndAddons, value);
  }

  public ItemToBeInstalled<NewConstructionOptions>? Nco
  {
    get => _nco;
    set => this.RaiseAndSetIfChanged(ref _nco, value);
  }

  public InstallGameViewModel(
    IScreen hostScreen,
    LauncherConfigService configService,
    GameDataService gameDataService,
    NcoReleaseService releaseService,
    PathsConfig paths
  ) : base("install-games", hostScreen)
  {
    _gameDataService = gameDataService;
    _releaseService = releaseService;
    _paths = paths;

    _installLog = string.Empty;

    IsInstalling = false;
    InstallFinished = false;
    HasErrored = false;

    SafeWhenNavigatedTo(() =>
    {
      DiscImages = configService.Config
        .EnabledDiscImageSources
        .Select(ItemToBeInstalled<DiscImageSource>.Build)
        .ToList();

      ModsAndAddons = configService.Config
        .EnabledZipUrlSpecs
        .Select(ItemToBeInstalled<ZipUrlSpec>.Build)
        .ToList();

      Nco = new ItemToBeInstalled<NewConstructionOptions>(configService.Config.Nco);

      return new CompositeDisposable(
        this.WhenValueChanged(x => x.InstallFinished)
          .Where(x => x)
          .ObserveOn(RxApp.MainThreadScheduler)
          .InvokeCommand(ReactiveCommand.Create<bool>(GoToLaunchGameScreen)),
        ReactiveCommand.CreateFromTask(Install)
          .Execute()
          .ObserveOn(RxApp.TaskpoolScheduler)
          .Subscribe()
      );
    });
  }

  private Task Install()
  {
    return Task.Run(async () =>
    {
      var downloadEventVisitor = new InstallDownloadEventVisitor(this);

      await _gameDataService.Download(
        downloadEventVisitor,
        b => CurrentSplashScreen = b
      );

      if (!HasErrored)
      {
        await _releaseService.Download(downloadEventVisitor);
      }

      InstallLog += HasErrored ? "Install failed to complete due to errors" : "Install complete";

      IsInstalling = false;
      InstallFinished = !HasErrored;

      await File.WriteAllTextAsync(Path.Join(_paths.NcoAppDataPath, "install.log"), InstallLog);
    });
  }

  private void GoToLaunchGameScreen(bool isInstalled)
  {
    if (!isInstalled)
    {
      return;
    }

    Nco!.Item.InstallPath = Nco.Item.PendingInstallPath;
    Nco.Item.Installed = isInstalled;

    if (OperatingSystem.IsMacOS())
    {
      // show user some guidance for using the app bundles
      HostScreen.Router.NavigateTo<MacOsGuideViewModel>();
      return;
    }
    
    HostScreen.Router.NavigateTo<LaunchGameViewModel>();
  }
}
