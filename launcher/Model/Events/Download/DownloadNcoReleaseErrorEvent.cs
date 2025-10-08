using System;

namespace CNC.NCO.Launcher.Model.Events.Download;

public record DownloadNcoReleaseErrorEvent(Exception Error) : IEvent<IDownloadEventVisitor>
{
  public void Accept(IDownloadEventVisitor visitor) => visitor.Visit(this);
}
