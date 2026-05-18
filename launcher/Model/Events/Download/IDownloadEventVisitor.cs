using System.Runtime.Versioning;

namespace CNC.NCO.Launcher.Model.Events.Download;

public interface IDownloadEventVisitor : IVisitor
{
  // Game data install
  void Visit(StartDownloadGameDataEvent e);
  void Visit(StartDiscImageDownloadEvent e);
  void Visit(FinishDiscImageDownloadEvent e);
  void Visit(ConvertDiscImageEvent e);
  void Visit(StartDiscImageFileScanEvent e);
  void Visit(WriteGameDataFileEvent e);
  void Visit(FinishDiscImageFileScanEvent e);
  void Visit(FinishDownloadGameDataEvent e);
  void Visit(DownloadFallbackEvent downloadFallbackEvent);
  void Visit(DownloadGameDataErrorEvent e);

  // Mods/addons install
  void Visit(StartZipUrlDownloadEvent e);
  void Visit(FinishZipUrlDownloadEvent e);

  // see: launcher/Service/NcoReleaseService.cs
  void Visit(FetchNcoReleaseEvent e);

  // NCO game engine install events
  void Visit(StartNcoReleaseDownloadEvent e);
  void Visit(FinishNcoReleaseDownloadEvent e);
  [SupportedOSPlatform("windows")]
  void Visit(FetchMsvcRuntimeEvent e);
  void Visit(ShortcutCreatedEvent e);
  void Visit(DownloadNcoReleaseErrorEvent e);

  // NCO launcher install events
  void Visit(StartNcoLauncherDownloadEvent e);
  void Visit(FinishNcoLauncherDownloadEvent e);
  void Visit(DownloadNcoLauncherErrorEvent e);
}
