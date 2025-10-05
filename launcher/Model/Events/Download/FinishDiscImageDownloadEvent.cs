namespace CNC.NCO.Launcher.Model.Events.Download;

public record FinishDiscImageDownloadEvent(DiscImageSource Image) : IEvent<IDownloadEventVisitor>
{
  public void Accept(IDownloadEventVisitor visitor) => visitor.Visit(this);
}
