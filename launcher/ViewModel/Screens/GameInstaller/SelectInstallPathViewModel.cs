using System.Linq;
using System.Reactive;
using System.Threading.Tasks;

using Avalonia.Platform.Storage;
using ReactiveUI;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Model;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller
{
  public class SelectInstallPathViewModel : ScreenViewModelBase
  {
    private readonly IStorageProvider _storageProvider;

    public NewConstructionOptions Nco { get; }
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
      Nco = configService.Config.NCO;

      if (string.IsNullOrWhiteSpace(Nco.InstallPath))
      {
        // default install path
        Nco.InstallPath = paths.AppDataPath;
      }

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
        SuggestedStartLocation = await _storageProvider.TryGetFolderFromPathAsync(Nco.InstallPath)
      });

      if (installFolders.Any())
      {
        Nco.InstallPath = installFolders[0].Path.AbsolutePath;
      }
    }
  }
}
