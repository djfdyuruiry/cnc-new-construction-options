using System;

namespace CNC.NCO.Launcher.Model.Events.Download;

public record DownloadGameDataErrorEvent(GameDataConfig? GameData, Exception Error) : IEvent<IDownloadEventVisitor>
{
  public void Accept(IDownloadEventVisitor visitor) => visitor.Visit(this);
}
