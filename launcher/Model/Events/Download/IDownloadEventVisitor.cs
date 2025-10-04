namespace CNC.NCO.Launcher.Model.Events.Download;

public interface IDownloadEventVisitor : IVisitor
{
  void Visit(ConvertDiscImageEvent e);
  void Visit(StartDiscImageDownloadEvent e);
  void Visit(WriteGameDataFileEvent e);
  void Visit(StartDownloadGameDataEvent e);
  void Visit(StartDiscImageFileScanEvent e);
}
