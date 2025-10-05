namespace CNC.NCO.Launcher.Model.Events.Download;

public interface IDownloadEventVisitor : IVisitor
{
  // Game data install
  void Visit(StartDownloadGameDataEvent e);
  void Visit(StartDiscImageDownloadEvent e);
  void Visit(FinishDiscImageDownloadEvent e);
  void Visit(ConvertDiscImageEvent e);
  void Visit(StartDiscImageFileScanEvent e);
  void Visit(WriteGameDataFileEvent e);
  void Visit(FinishDiscImageFileScanEvent e);
  void Visit(FinishDownloadGameDataEvent e);
  void Visit(DownloadGameDataErrorEvent e);

  // TODO: zip URL events

  // NCO game engine install events, see: launcher/Service/NcoReleaseService.cs
  void Visit(FetchNcoReleaseEvent e);
  void Visit(StartNcoReleaseDownloadEvent e);
  void Visit(FinishNcoReleaseDownloadEvent e);
  void Visit(DownloadNcoReleaseErrorEvent e);
}
