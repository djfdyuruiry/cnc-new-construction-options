namespace CNC.NCO.Launcher.Model.Events.Download;

public record ShortcutCreatedEvent(GameDataConfig game): IEvent<IDownloadEventVisitor>
{
  public void Accept(IDownloadEventVisitor visitor) => visitor.Visit(this);
}
