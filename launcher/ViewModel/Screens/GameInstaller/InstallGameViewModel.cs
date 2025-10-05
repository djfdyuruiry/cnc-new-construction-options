using System.Reactive;
using CNC.NCO.Launcher.Model.Events.Download;
using CNC.NCO.Launcher.Service;
using ReactiveUI;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

public class InstallGameViewModel : ReactiveObject
{
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
  }

  private readonly GameDataService _gameDataService;
  private readonly DownloadEventVisitor _downloadEventVisitor;
  private string _installLog;
  
  public ReactiveCommand<Unit, Unit> Install { get; }

  public string InstallLog
  {
    get => _installLog;
    set => this.RaiseAndSetIfChanged(ref _installLog, value);
  }

  public InstallGameViewModel(GameDataService gameDataService)
  {
    _gameDataService = gameDataService;
    _downloadEventVisitor = new DownloadEventVisitor(this);

    Install = ReactiveCommand.CreateFromTask(() =>
      _gameDataService.Download(_downloadEventVisitor, b => { })
    );
  }
}
