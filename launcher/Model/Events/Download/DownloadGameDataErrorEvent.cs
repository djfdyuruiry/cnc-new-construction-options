namespace CNC.NCO.Launcher.Model.Events.Download;

public record DownloadGameDataErrorEvent(GameDataConfig GameData, string Error) : IEvent<IDownloadEventVisitor>
{
  public void Accept(IDownloadEventVisitor visitor) => visitor.Visit(this);
}
