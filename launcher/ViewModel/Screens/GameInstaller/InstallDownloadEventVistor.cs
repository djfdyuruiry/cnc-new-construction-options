using System;
using System.Linq;
using System.Runtime.Versioning;

using CNC.NCO.Launcher.Model;
using CNC.NCO.Launcher.Model.Events.Download;
using CNC.NCO.Launcher.Model.ViewModel;

namespace CNC.NCO.Launcher.ViewModel.Screens.GameInstaller;

internal sealed class InstallDownloadEventVisitor(InstallGameViewModel host) : IDownloadEventVisitor
{
  private void AppendToInstallLog(string line)
  {
    host.InstallLog += $"{line}\n";
  }

  private ItemToBeInstalled<DiscImageSource>? _currentDiscImage;
  private ItemToBeInstalled<ZipUrlSpec>? _currentZip;

  // game data install
  public void Visit(ConvertDiscImageEvent e) =>
    AppendToInstallLog($"Converting disc image to ISO format: {e.Image.Config.File}");

  public void Visit(StartDiscImageDownloadEvent e)
  {
    AppendToInstallLog($"Downloading disc image {e.Image.Config.File} from source: {e.Image.Config.DisplayName}");

    _currentDiscImage = host.DiscImages!.FirstOrDefault(d => Equals(d.Item, e.Image));

    if (_currentDiscImage is null)
    {
      Console.Error.WriteLine(
        $"WARN: {nameof(StartDiscImageFileScanEvent)} received with disc image {e.Image.Config.File} " +
        $"that was not found in view model property '{nameof(host.DiscImages)}'"
      );
      return;
    }

    _currentDiscImage.Installing = true;
  }

  public void Visit(WriteGameDataFileEvent e) =>
    AppendToInstallLog($"Writing game data file: {e.File} to {e.DestPath}");

  public void Visit(StartDownloadGameDataEvent e) =>
    AppendToInstallLog($"Starting data download(s) for game: {e.GameData.DisplayName}");

  public void Visit(StartDiscImageFileScanEvent e)
  {
    AppendToInstallLog($"Scanning files in disc image source: {e.Source.Config.File}");

    _currentDiscImage = host.DiscImages!.FirstOrDefault(d => Equals(d.Item, e.Source));

    if (_currentDiscImage is null)
    {
      Console.Error.WriteLine(
        $"WARN: {nameof(StartDiscImageFileScanEvent)} received with disc image {e.Source.Config.File} " +
        $"that was not found in view model property '{nameof(host.DiscImages)}'"
      );
      return;
    }

    _currentDiscImage.Installing = true;
  }

  public void Visit(FinishDiscImageDownloadEvent e) =>
    AppendToInstallLog($"Game disc downloaded: {e.Image.DisplayNameOrName}");

  public void Visit(FinishDiscImageFileScanEvent e)
  {
    AppendToInstallLog($"Disc image file scan complete: {e.Source.Config.File}");

    if (_currentDiscImage is null)
    {
      Console.Error.WriteLine(
        $"WARN: {nameof(FinishDiscImageFileScanEvent)} received but {nameof(_currentDiscImage)} is null"
      );
      return;
    }

    _currentDiscImage.Installing = false;
    _currentDiscImage.Installed = true;
    _currentDiscImage.Errored = false;
  }

  public void Visit(FinishDownloadGameDataEvent e) =>
    AppendToInstallLog($"Game data install complete: {e.GameData.DisplayName}");

  // mods/addons install
  public void Visit(StartZipUrlDownloadEvent e)
  {
    AppendToInstallLog($"Starting zip url download: {e.Spec.Url}");

    _currentZip = host.ModsAndAddons!.FirstOrDefault(z => Equals(z.Item, e.Spec));

    if (_currentZip is null)
    {
      Console.Error.WriteLine(
        $"WARN: {nameof(StartZipUrlDownloadEvent)} received with zip spec {e.Spec.DisplayNameOrName} " +
        $"that was not found in view model property '{nameof(host.ModsAndAddons)}'"
      );
      return;
    }

    _currentZip.Installing = true;
  }

  public void Visit(FinishZipUrlDownloadEvent e)
  {
    AppendToInstallLog($"Zip url download complete: {e.Spec.Url}");

    if (_currentZip is null)
    {
      Console.Error.WriteLine(
        $"WARN: {nameof(FinishZipUrlDownloadEvent)} received but {nameof(_currentZip)} is null"
      );
      return;
    }

    _currentZip.Installing = false;
    _currentZip.Installed = true;
    _currentZip.Errored = false;
  }

  // nco install
  public void Visit(FetchNcoReleaseEvent e)
  {
    AppendToInstallLog($"Fetching info on the latest version of NCO game engine");

    host.Nco!.Installing = true;
  }

  public void Visit(StartNcoReleaseDownloadEvent e) =>
    AppendToInstallLog($"Downloading NCO game engine");

  public void Visit(FinishNcoReleaseDownloadEvent e)
  {
    AppendToInstallLog($"NCO game engine installed");

    host.Nco!.Installed = true;
    host.Nco.Installing = false;
    host.Nco.Errored = false;
  }

  [SupportedOSPlatform("windows")]
  public void Visit(FetchMsvcRuntimeEvent e) =>
    AppendToInstallLog("Ensuring MSVC runtime is installed - please accept any UAC prompts");

  public void Visit(ShortcutCreatedEvent e) =>
    AppendToInstallLog($"Created desktop shortcut for game '{e.game.DisplayName}'");

  // error handling
  public void Visit(DownloadFallbackEvent e)
  {
    AppendToInstallLog($"Primary game data source '{e.FailedSource.DisplayName}' failed, falling back to " +
                       $"secondary source '{e.FallbackSource.DisplayName}': {e.Error}");
  }

  public void Visit(DownloadGameDataErrorEvent e)
  {
    var gamePlaceholder = e.GameData is not null ? $" '{e.GameData!.DisplayName}'" : string.Empty;
    AppendToInstallLog($"Error installing{gamePlaceholder} game data: {e.Error}");

    host.HasErrored = true;

    if (_currentZip is not null)
    {
      _currentZip.Installing = false;
      _currentZip.Installed = false;
      _currentZip.Errored = true;
      return;
    }

    if (_currentDiscImage is null)
    {
      Console.Error.WriteLine(
        $"WARN: {nameof(DownloadGameDataErrorEvent)} received but {nameof(_currentDiscImage)} is null"
      );
      return;
    }

    _currentDiscImage.Installing = false;
    _currentDiscImage.Installed = false;
    _currentDiscImage.Errored = true;
  }

  public void Visit(DownloadNcoReleaseErrorEvent e)
  {
    AppendToInstallLog($"Error installing NCO game engine: {e.Error}");

    host.HasErrored = true;

    host.Nco!.Errored = true;
    host.Nco.Installing = false;
    host.Nco.Installed = false;
  }
}
