namespace CNC.NCO.Launcher.Model.Events.Download;

public interface IDownloadEventVisitor : IVisitor
{
  void Visit(ConvertDiscImageEvent e);
  void Visit(StartDiscImageDownloadEvent e);
  void Visit(WriteGameDataFileEvent e);
  void Visit(StartDownloadGameDataEvent e);
  void Visit(StartDiscImageFileScanEvent e);
  void Visit(FinishDiscImageDownloadEvent e);
  void Visit(FinishDownloadGameDataEvent e);
  void Visit(FetchNcoReleaseEvent e);
  void Visit(DownloadGameDataErrorEvent e);
  void Visit(StartNcoReleaseDownloadEvent e);
  void Visit(DownloadNcoReleaseErrorEvent e);
}
