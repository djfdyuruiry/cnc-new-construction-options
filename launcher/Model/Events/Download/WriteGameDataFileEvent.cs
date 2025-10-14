namespace CNC.NCO.Launcher.Model.Events.Download;

public record WriteGameDataFileEvent(string File, string DestPath) : IEvent<IDownloadEventVisitor>
{
  public void Accept(IDownloadEventVisitor visitor) => visitor.Visit(this);
}
