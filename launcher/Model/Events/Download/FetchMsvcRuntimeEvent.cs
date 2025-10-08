using System.Runtime.Versioning;

namespace CNC.NCO.Launcher.Model.Events.Download;

[SupportedOSPlatform("windows")]
public record FetchMsvcRuntimeEvent() : IEvent<IDownloadEventVisitor>
{
  public void Accept(IDownloadEventVisitor visitor) => visitor.Visit(this);
}
