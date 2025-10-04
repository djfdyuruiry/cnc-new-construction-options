namespace CNC.NCO.Launcher.Model.Events.Download;

public record ConvertDiscImageEvent(DiscImageSource Image, string SourceFormat, string TargetFormat) : IEvent<IDownloadEventVisitor>
{
  public void Accept(IDownloadEventVisitor visitor) => visitor.Visit(this);
}
