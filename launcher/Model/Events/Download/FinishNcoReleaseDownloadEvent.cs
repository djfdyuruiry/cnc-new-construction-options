namespace CNC.NCO.Launcher.Model.Events.Download;

public record FinishNcoReleaseDownloadEvent() : IEvent<IDownloadEventVisitor>
{
  public void Accept(IDownloadEventVisitor visitor) => visitor.Visit(this);
}
