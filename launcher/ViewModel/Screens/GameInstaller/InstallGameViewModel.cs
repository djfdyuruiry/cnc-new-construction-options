using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive;

using Avalonia.Media;
using ReactiveUI;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Model.ViewModel;
using CNC.NCO.Launcher.Service;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

public class InstallGameViewModel : ScreenViewModelBase
{
  private string _installLog;
  private IBrush? _backgroundImage;
  private bool _installNotClicked;
  private bool _isInstalling;
  private bool _installFinished;
  private ObservableCollection<DiscImageToBeInstalled> _enabledDiscImages;
  private bool _ncoInstalling;
  private bool _ncoInstalled;
  private bool _ncoErrored;

  public ObservableCollection<DiscImageToBeInstalled> DiscImages
  {
    get => _enabledDiscImages;
    set => this.RaiseAndSetIfChanged(ref _enabledDiscImages, value);
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

  public bool InstallFinished
  {
    get => _installFinished;
    set => this.RaiseAndSetIfChanged(ref _installFinished, value);
  }

  public bool NcoInstalling
  {
    get => _ncoInstalling;
    set => this.RaiseAndSetIfChanged(ref _ncoInstalling, value);
  }

  public bool NcoInstalled
  {
    get => _ncoInstalled;
    set => this.RaiseAndSetIfChanged(ref _ncoInstalled, value);
  }

  public bool NcoErrored
  {
    get => _ncoErrored;
    set => this.RaiseAndSetIfChanged(ref _ncoErrored, value);
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
    _installLog = string.Empty;
    _installNotClicked = true;
    _isInstalling = false;
    _installFinished = false;

    _enabledDiscImages = new ObservableCollection<DiscImageToBeInstalled>(
      configService.Config
        .Games
        .SelectMany(
          // TODO: Allow user to select source and pass into this method to filter (instead of first)
          g => g.DiscImagesBySource
            .First()
            .Value
            .Where(d => d.Enabled)
        )
        .Select(d => new DiscImageToBeInstalled(d))
        .OrderBy(d => d.Source.Game.SortOrder)
        .ThenBy(d => d.Source.SortOrder)
    );

    // TODO: add NCO release download here
    Start = ReactiveCommand.CreateFromTask(async () =>
    {
      InstallNotClicked = true;
      IsInstalling = false;
      InstallFinished = false;

      var downloadEventVisitor = new InstallDownloadEventVisitor(this);

      await gameDataService.Download(
        downloadEventVisitor,
        b => BackgroundImage = new ImageBrush(b)
      );

      await releaseService.Download(downloadEventVisitor);

      IsInstalling = false;
      InstallFinished = true;
    });

    Finish = ReactiveCommand.CreateFromObservable(() =>
      HostScreen.Router.NavigateTo<LaunchGameViewModel>()
    );
  }
}
