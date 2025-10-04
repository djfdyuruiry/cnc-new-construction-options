using System;
using System.IO;
using System.Linq;
using System.Reactive;
using System.Threading.Tasks;

using Avalonia.Platform.Storage;
using ReactiveUI;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller
{
    public class SelectInstallPathViewModel : ScreenViewModelBase
    {
      private readonly IStorageProvider _storageProvider;
      private string _installPath;

        public string InstallPath
        {
            get => _installPath;
            set => this.RaiseAndSetIfChanged(ref _installPath, value);
        }

        public ReactiveCommand<Unit, Unit> Browse { get; }
        public ReactiveCommand<Unit, IRoutableViewModel> Next { get; }

        public SelectInstallPathViewModel(IScreen hostScreen, MainWindow hostWindow)
            : base("select-install-path", hostScreen)
        {
          _storageProvider = hostWindow.StorageProvider;

          InstallPath = Path.Join(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "nco");

          Browse = ReactiveCommand.CreateFromTask(BrowseForInstallPath);
          Next = ReactiveCommand.CreateFromObservable(() => 
            HostScreen.Router.NavigateTo<SelectGameDataViewModel>()
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
