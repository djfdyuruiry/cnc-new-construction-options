using System.Reactive;

using Avalonia.Media;
using ReactiveUI;

using CNC.NCO.Launcher.Model.Events.Download;
using CNC.NCO.Launcher.Service;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

// TODO: impl view model and bind
public class InstallGameViewModel : ReactiveObject
{
  // TODO: add list of disc images and add gif spinner whilst installing, with check mark when done - red x when error
  private sealed class DownloadEventVisitor(InstallGameViewModel parent) : IDownloadEventVisitor
  {
    private void AppendToInstallLog(string line)
    {
      parent._installLog += $"{line}\n";
    }

    public void Visit(ConvertDiscImageEvent e) =>
      AppendToInstallLog($"Converting disc image to ISO format: {e.Image.Config.File}");

    public void Visit(StartDiscImageDownloadEvent e) =>
      AppendToInstallLog($"Downloading disc image: {e.Image.Config.File}");

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

    public void Visit(DownloadGameDataErrorEvent downloadGameDataErrorEvent)
    {
      throw new System.NotImplementedException();
    }

    public void Visit(StartNcoReleaseDownloadEvent e)
    {
      throw new System.NotImplementedException();
    }

    public void Visit(DownloadNcoReleaseErrorEvent e)
    {
      throw new System.NotImplementedException();
    }

    public void Visit(FinishDiscImageDownloadEvent downloadGameDataErrorEvent)
    {
      throw new System.NotImplementedException();
    }

    public void Visit(FinishDownloadGameDataEvent downloadGameDataErrorEvent)
    {
      throw new System.NotImplementedException();
    }
  }

  private string _installLog;
  private IBrush? _backgroundImage;
  
  public ReactiveCommand<Unit, Unit> Install { get; }

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

  public InstallGameViewModel(GameDataService gameDataService, NcoReleaseService releaseService)
  {
    var downloadEventVisitor = new DownloadEventVisitor(this);

    // TODO: add NCO release download here
    Install = ReactiveCommand.CreateFromTask(async () =>
    {
      await gameDataService.Download(
        downloadEventVisitor,
        b => BackgroundImage = new ImageBrush(b)
      );

      await releaseService.Download(downloadEventVisitor);
    });
}
}
