namespace CNC.NCO.Launcher.Model.Events.Download;

public record FinishZipUrlDownloadEvent(ZipUrlSpec Spec) : IEvent<IDownloadEventVisitor>
{
  public void Accept(IDownloadEventVisitor visitor) => visitor.Visit(this);
}
