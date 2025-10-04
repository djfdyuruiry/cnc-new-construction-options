namespace CNC.NCO.Launcher.Model.Events.Download;

public record StartDownloadGameDataEvent(GameDataConfig GameData) : IEvent<IDownloadEventVisitor>
{
  public void Accept(IDownloadEventVisitor visitor) => visitor.Visit(this);
}
