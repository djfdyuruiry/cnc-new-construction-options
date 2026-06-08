using System;
using System.Runtime.Versioning;

using CNC.NCO.Launcher.Model.Events.Download;

namespace CNC.NCO.Launcher;

internal class SilentInstallDownloadVisitor : IDownloadEventVisitor
{
  private Action<string>? _installLogCallback = null;

  public bool HasErrored { get; private set; }
  public string? Error { get; private set; }

  public void SetInstallLogCallback(Action<string> callback)
  {
    _installLogCallback = callback;
  }

  private void LogInstallMessage(string message)
  {
    _installLogCallback!.Invoke(message + Environment.NewLine);
    Console.WriteLine(message);
  }

  private void LogInstallErrorMessage(string message)
  {
    _installLogCallback!.Invoke(message + Environment.NewLine);
    Console.Error.WriteLine(message);
  }

  // game data install
  public void Visit(ConvertDiscImageEvent e) =>
    LogInstallMessage($"Converting disc image to ISO format: {e.Image.Config.File}");

  public void Visit(StartDiscImageDownloadEvent e) => 
    LogInstallMessage($"Downloading disc image {e.Image.Config.File} from source: {e.Image.Config.DisplayName}");

  public void Visit(WriteGameDataFileEvent e) =>
    LogInstallMessage($"Writing game data file: {e.File} to {e.DestPath}");

  public void Visit(StartDownloadGameDataEvent e) =>
    LogInstallMessage($"Starting data download(s) for game: {e.GameData.DisplayName}");

  public void Visit(StartDiscImageFileScanEvent e) => 
    LogInstallMessage($"Scanning files in disc image source: {e.Source.Config.File}");

  public void Visit(FinishDiscImageDownloadEvent e) =>
    LogInstallMessage($"Game disc downloaded: {e.Image.DisplayNameOrName}");

  public void Visit(FinishDiscImageFileScanEvent e) => 
    LogInstallMessage($"Disc image file scan complete: {e.Source.Config.File}");

  public void Visit(FinishDownloadGameDataEvent e) =>
    LogInstallMessage($"Game data install complete: {e.GameData.DisplayName}");

  // mods/addons install
  public void Visit(StartZipUrlDownloadEvent e) => LogInstallMessage($"Starting zip url download: {e.Spec.Url}");

  public void Visit(FinishZipUrlDownloadEvent e) => LogInstallMessage($"Zip url download complete: {e.Spec.Url}");

  // error handling
  public void Visit(DownloadFallbackEvent e) =>
    LogInstallErrorMessage($"Primary game data source '{e.FailedSource.DisplayName}' failed, falling back to " +
                      $"secondary source '{e.FallbackSource.DisplayName}': {e.Error}");

  public void Visit(DownloadGameDataErrorEvent e)
  {
    var gamePlaceholder = e.GameData is not null ? $" '{e.GameData!.DisplayName}'" : string.Empty;
    LogInstallErrorMessage($"Error installing{gamePlaceholder} game data: {e.Error}");

    Error = e.Error.Message;
    HasErrored = true;
  }

  // nco github release lookup
  public void Visit(FetchNcoReleaseEvent e) => 
    LogInstallMessage($"Fetching info on the latest version of NCO game engine");

  // game engine install
  public void Visit(StartNcoReleaseDownloadEvent e) => LogInstallMessage($"Downloading NCO game engine");

  [SupportedOSPlatform("windows")]
  public void Visit(FetchMsvcRuntimeEvent e) => 
    LogInstallMessage("Ensuring MSVC runtime is installed - please accept any UAC prompts");

  public void Visit(ShortcutCreatedEvent e) => LogInstallMessage($"Created desktop shortcut for '{e.Name}'");

  public void Visit(FinishNcoReleaseDownloadEvent e) => LogInstallMessage($"NCO game engine installed");

  public void Visit(DownloadNcoReleaseErrorEvent e)
  {
    LogInstallErrorMessage($"Error installing NCO game engine: {e.Error}");

    Error = e.Error.Message;
    HasErrored = true;
  }

  // launcher install
  public void Visit(StartNcoLauncherDownloadEvent e) => LogInstallErrorMessage($"Downloading NCO Launcher");

  public void Visit(FinishNcoLauncherDownloadEvent e) => LogInstallErrorMessage($"NCO Launcher installed");

  public void Visit(DownloadNcoLauncherErrorEvent e) => 
    LogInstallErrorMessage($"Error installing NCO Launcher: {e.Error}");
}
