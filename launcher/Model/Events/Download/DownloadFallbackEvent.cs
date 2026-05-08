using System;

namespace CNC.NCO.Launcher.Model.Events.Download;

public record DownloadFallbackEvent(
  GameDataConfig GameData,
  DiscImageSourceConfig FailedSource,
  DiscImageSourceConfig FallbackSource,
  Exception Error
) : IEvent<IDownloadEventVisitor>
{
  public void Accept(IDownloadEventVisitor visitor) => visitor.Visit(this);
}
