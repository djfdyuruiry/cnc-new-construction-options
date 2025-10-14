namespace CNC.NCO.Launcher.Model.Events.Download;

public record StartZipUrlDownloadEvent(ZipUrlSpec Spec) : IEvent<IDownloadEventVisitor>
{
  public void Accept(IDownloadEventVisitor visitor) => visitor.Visit(this);
}
