namespace CNC.NCO.Launcher.Model.Events.Download;

public record StartDiscImageFileScanEvent(DiscImageSource Source) : IEvent<IDownloadEventVisitor>
{
  public void Accept(IDownloadEventVisitor visitor) => visitor.Visit(this);
}
