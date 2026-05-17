namespace CNC.NCO.Launcher.Model.Events.Download;

public record FinishNcoLauncherDownloadEvent() : IEvent<IDownloadEventVisitor>
{
  public void Accept(IDownloadEventVisitor visitor) => visitor.Visit(this);
}
