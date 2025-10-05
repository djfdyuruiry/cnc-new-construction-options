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

  public void Visit(StartDiscImageDownloadEvent e)
  {
    AppendToInstallLog($"Downloading disc image: {e.Image.Config.File}");
    _currentDiscImage = host.DiscImages.FirstOrDefault(d => Equals(d.Source, e.Image));

    if (_currentDiscImage is null)
    {
      // TODO: log warning
      return;
    }
      
    _currentDiscImage.Installing = true;
  }

  public void Visit(WriteGameDataFileEvent e) =>
    AppendToInstallLog($"Writing game data file: {e.File} to {e.DestPath}");

  public void Visit(StartDownloadGameDataEvent e) =>
    AppendToInstallLog($"Starting data download(s) for game: {e.GameData.DisplayName}");

  public void Visit(StartDiscImageFileScanEvent e) =>
    AppendToInstallLog($"Scanning files in disc image source: {e.Source.Config.File}");

  public void Visit(FetchNcoReleaseEvent e)
  {
    throw new System.NotImplementedException();
  }

  public void Visit(StartNcoReleaseDownloadEvent e)
  {
    throw new System.NotImplementedException();
  }

  public void Visit(FinishDiscImageDownloadEvent downloadGameDataErrorEvent)
  {
    throw new System.NotImplementedException();
  }

  public void Visit(FinishDiscImageFileScanEvent finishDiscImageFileScanEvent)
  {
    AppendToInstallLog($"Disc image file scan complete: {finishDiscImageFileScanEvent.Source.Config.File}");

    if (_currentDiscImage is null)
    {
      // TODO: log warning
      return;
    }

    _currentDiscImage.Installing = false;
    _currentDiscImage.Installed = true;
    _currentDiscImage.Errored = false;
  }

  public void Visit(FinishDownloadGameDataEvent downloadGameDataErrorEvent)
  {
    throw new System.NotImplementedException();
  }

  // error handling
  public void Visit(DownloadGameDataErrorEvent e)
  {
    AppendToInstallLog($"Error installing '{e.GameData.DisplayName}': {e.Error}");

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
    throw new System.NotImplementedException();
  }
}
