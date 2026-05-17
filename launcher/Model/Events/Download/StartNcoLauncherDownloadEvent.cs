namespace CNC.NCO.Launcher.Model.Events.Download;

public record StartNcoLauncherDownloadEvent(string AssetUrl) : IEvent<IDownloadEventVisitor>
{
  public void Accept(IDownloadEventVisitor visitor) => visitor.Visit(this);
}
