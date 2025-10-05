using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive;

using Avalonia.Media;
using ReactiveUI;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Model.Events.Download;
using CNC.NCO.Launcher.Model.ViewModel;
using CNC.NCO.Launcher.Service;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

// TODO: impl view model and bind
public class InstallGameViewModel : ScreenViewModelBase
{
  private string _installLog;
  private IBrush? _backgroundImage;
  private bool _isInstalling;
  private bool _installFinished;
  private ObservableCollection<DiscImageToBeInstalled> _enabledDiscImages;

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

  public ReactiveCommand<Unit, Unit> Install { get; }
  public ReactiveCommand<Unit, IRoutableViewModel> Next { get; }

  public InstallGameViewModel(
    IScreen hostScreen,
    LauncherConfigService configService,
    GameDataService gameDataService,
    NcoReleaseService releaseService
  )
    : base("install-games", hostScreen)
  {
    _installLog = string.Empty;
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
    Install = ReactiveCommand.CreateFromTask(async () =>
    {
      IsInstalling = true;
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

    Next = ReactiveCommand.CreateFromObservable(() =>
      HostScreen.Router.NavigateTo<LaunchGameViewModel>()
    );
  }
}
