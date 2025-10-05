using System.Linq;
using System.Reactive;
using System.Threading.Tasks;

using Avalonia.Platform.Storage;
using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Model;
using ReactiveUI;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller
{
  public class SelectInstallPathViewModel : ScreenViewModelBase
  {
    private readonly IStorageProvider _storageProvider;
    private readonly NewConstructionOptions _ncoConfig;
    private string _installPath;

    public string InstallPath
    {
      get => _installPath;
      set
      {
        this.RaiseAndSetIfChanged(ref _installPath, value); 
        _ncoConfig.InstallPath = InstallPath;
      }
    }

    public ReactiveCommand<Unit, Unit> Browse { get; }
    public ReactiveCommand<Unit, IRoutableViewModel> Next { get; }

    public SelectInstallPathViewModel(
      IScreen hostScreen,
      LauncherConfigService configService,
      PathsConfig paths,
      MainWindow hostWindow
    ) : base("select-install-path", hostScreen)
    {
      _storageProvider = hostWindow.StorageProvider;
      _ncoConfig = configService.Config.NCO;
      _installPath = _ncoConfig.InstallPath ?? paths.AppDataPath;

      Browse = ReactiveCommand.CreateFromTask(BrowseForInstallPath);
      Next = ReactiveCommand.CreateFromObservable(() =>
        HostScreen.Router.NavigateTo<InstallGameViewModel>()
      );
    }

    private async Task BrowseForInstallPath()
    {
      var installFolders = await _storageProvider.OpenFolderPickerAsync(new FolderPickerOpenOptions
      {
        AllowMultiple = false,
        Title = "Select Install Path",
        SuggestedStartLocation = await _storageProvider.TryGetFolderFromPathAsync(InstallPath)
      });

      if (installFolders.Any())
      {
        InstallPath = installFolders[0].Path.AbsolutePath;
      }
    }
  }
}
