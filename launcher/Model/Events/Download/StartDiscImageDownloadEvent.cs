namespace CNC.NCO.Launcher.Model.Events.Download;

public record StartDiscImageDownloadEvent(DiscImageSource Image) : IEvent<IDownloadEventVisitor>
{
  public void Accept(IDownloadEventVisitor visitor) => visitor.Visit(this);
}
