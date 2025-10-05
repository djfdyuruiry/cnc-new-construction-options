using System.Linq;

using CNC.NCO.Launcher.Model.Events.Download;
using CNC.NCO.Launcher.Model.ViewModel;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

internal sealed class InstallDownloadEventVisitor(InstallGameViewModel host) : IDownloadEventVisitor
{
  private void AppendToInstallLog(string line)
  {
    host.InstallLog += $"{line}\n";
  }

  private DiscImageToBeInstalled? _currentDiscImage;

  public void Visit(ConvertDiscImageEvent e) =>
    AppendToInstallLog($"Converting disc image to ISO format: {e.Image.Config.File}");

  public void Visit(StartDiscImageDownloadEvent e) => 
    AppendToInstallLog($"Downloading disc image: {e.Image.Config.File}");

  public void Visit(WriteGameDataFileEvent e) =>
    AppendToInstallLog($"Writing game data file: {e.File} to {e.DestPath}");

  public void Visit(StartDownloadGameDataEvent e) =>
    AppendToInstallLog($"Starting data download(s) for game: {e.GameData.DisplayName}");

  public void Visit(StartDiscImageFileScanEvent e)
  {
    AppendToInstallLog($"Scanning files in disc image source: {e.Source.Config.File}");
    
    _currentDiscImage = host.DiscImages.FirstOrDefault(d => Equals(d.Source, e.Source));

    if (_currentDiscImage is null)
    {
      // TODO: log warning
      return;
    }
      
    _currentDiscImage.Installing = true;
  }

  public void Visit(FetchNcoReleaseEvent e)
  {
    AppendToInstallLog($"Fetching info on the latest version of NCO game engine");

    host.NcoInstalling = true;
  }

  public void Visit(StartNcoReleaseDownloadEvent e) => 
    AppendToInstallLog($"Downloading NCO game engine");

  public void Visit(FinishNcoReleaseDownloadEvent e)
  {
    AppendToInstallLog($"NCO game engine installed");

    host.NcoInstalled = true;
    host.NcoInstalling = false;
    host.NcoErrored = false;
  }

  public void Visit(FinishDiscImageDownloadEvent e) => 
    AppendToInstallLog($"Game disc downloaded: {e.Image.DisplayNameOrName}");

  public void Visit(FinishDiscImageFileScanEvent e)
  {
    AppendToInstallLog($"Disc image file scan complete: {e.Source.Config.File}");

    if (_currentDiscImage is null)
    {
      // TODO: log warning
      return;
    }

    _currentDiscImage.Installing = false;
    _currentDiscImage.Installed = true;
    _currentDiscImage.Errored = false;
  }

  public void Visit(FinishDownloadGameDataEvent e) => 
    AppendToInstallLog($"Game data install complete: {e.GameData.DisplayName}");

  // error handling
  public void Visit(DownloadGameDataErrorEvent e)
  {
    AppendToInstallLog($"Error installing '{e.GameData.DisplayName}' game data: {e.Error}");

    if (_currentDiscImage is null)
    {
      // TODO: log warning
      return;
    }
  
    _currentDiscImage.Installing = false;
    _currentDiscImage.Installed = false;
    _currentDiscImage.Errored = true;
  }

  public void Visit(DownloadNcoReleaseErrorEvent e)
  {
    AppendToInstallLog($"Error installing NCO game engine: {e.Error}");

    host.NcoErrored = true;
    host.NcoInstalling = false;
    host.NcoInstalled = false;
  }
}
