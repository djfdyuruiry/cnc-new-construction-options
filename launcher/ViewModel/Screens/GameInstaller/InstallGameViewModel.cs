using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reactive.Disposables;
using System.Reactive.Linq;
using System.Threading.Tasks;

using Avalonia.Media;
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
  private IBrush? _backgroundImage;
  private bool _hasErrored;
  private bool _installFinished;

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

  public IBrush? BackgroundImage
  {
    get => _backgroundImage;
    set => this.RaiseAndSetIfChanged(ref _backgroundImage, value);
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

  public IList<ItemToBeInstalled<DiscImageSource>> DiscImages { get; }

  public IList<ItemToBeInstalled<ZipUrlSpec>> ModsAndAddons { get; }
  
  public ItemToBeInstalled<NewConstructionOptions> Nco { get; }

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

    IsInstalling = false;
    InstallFinished = false;
    HasErrored = false;

    DiscImages = configService.Config.EnabledDiscImageSources 
      .Select(ItemToBeInstalled<DiscImageSource>.Build)
      .ToList();
    ModsAndAddons = configService.Config.EnabledZipUrlSpecs
      .Select(ItemToBeInstalled<ZipUrlSpec>.Build)
      .ToList();
    Nco = new ItemToBeInstalled<NewConstructionOptions>(configService.Config.NCO);

    this.WhenNavigatedTo(() =>
      new CompositeDisposable(
        this.WhenValueChanged(x => x.InstallFinished)
          .Where(x => x)
          .InvokeCommand(ReactiveCommand.Create<bool>(GoToLaunchGameScreen)),
        ReactiveCommand.CreateFromTask(Install)
          .Execute()
          .ObserveOn(RxApp.TaskpoolScheduler)
          .Subscribe()
      )
    );
  }

  private async Task Install()
  {
    var downloadEventVisitor = new InstallDownloadEventVisitor(this);

    await _gameDataService.Download(
      Nco.Item.PendingInstallPath,
      downloadEventVisitor,
      b => BackgroundImage = new ImageBrush(b)
    );
    await _releaseService.Download(Nco.Item.PendingInstallPath, downloadEventVisitor);

    IsInstalling = false;
    InstallFinished = !HasErrored;

    await File.WriteAllTextAsync(Path.Join(_paths.AppDataPath, "install.log"), InstallLog);
  }

  private void GoToLaunchGameScreen(bool isInstalled)
  {
    if (!isInstalled)
    {
      return;
    }

    Nco.Item.InstallPath = Nco.Item.PendingInstallPath;
    Nco.Item.Installed = isInstalled;

    HostScreen.Router.NavigateTo<LaunchGameViewModel>();
  }
}
