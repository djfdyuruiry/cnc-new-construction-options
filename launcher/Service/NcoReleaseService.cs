using System;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Runtime.Versioning;
using System.Threading;
using System.Threading.Tasks;

using GitHub;
using SharpCompress.Common;
using SharpCompress.Readers;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Model;
using CNC.NCO.Launcher.Model.Events.Download;
using CNC.NCO.Launcher.Util;

namespace CNC.NCO.Launcher.Service;

public class NcoReleaseService(LauncherConfigService configService, GitHubClient gitHubClient, PathsConfig pathsConfig)
{
  [SupportedOSPlatform("windows")]
  public async Task GenerateWindowsShortcuts(string installRoot, IDownloadEventVisitor eventVisitor)
  {
    var winUtils = new WindowsUtils(pathsConfig);

    foreach (var game in configService.Config.EnabledGames)
    {
      var gameBinaryPath = $"{Path.Join(installRoot, game.InstallPostfix, game.Binary)}.exe";

      await winUtils.CreateShortcut(
        $"{game.DisplayName.Replace("Command & Conquer:", "C&C -")} (NCO)",
        gameBinaryPath
      );

      eventVisitor.Visit(new ShortcutCreatedEvent(game));
    }
  }

  /**
   * Windows binaries require MSVC 2022 runtime to run, so we install it for the
   * user here.
   */
  [SupportedOSPlatform("windows")]
  private async Task InstallMsvcRuntime(IDownloadEventVisitor eventVisitor)
  {
    if (configService.Config.Nco.Installed)
    {
      // only needs installed once, so skip on subsequent installer executions
      return;
    }

    eventVisitor.Visit(new FetchMsvcRuntimeEvent());

    var winUtils = new WindowsUtils(pathsConfig);

    await winUtils.InstallMsvcRuntime(configService.Config.Nco.MsvcRuntimeUrl);
  }

  [SupportedOSPlatform("linux")]
  private async Task GenerateDesktopFiles(string installRoot, IDownloadEventVisitor eventVisitor)
  {
    var desktopTemplate = await File.ReadAllTextAsync(
      Path.Join(pathsConfig.ToolsPath, "nco.desktop")
    );
    var appsPath = Path.Join(pathsConfig.AppDataDirectoryPath, "applications");

    Directory.CreateDirectory(appsPath);

    foreach (var game in configService.Config.EnabledGames)
    {
      var gamePath = Path.Join(installRoot, game.InstallPostfix);
      var binaryPath = Path.Join(gamePath, game.PlatformBinary);
      var desktopName = $"nco-{game.InstallPostfix}";

      await File.WriteAllTextAsync(
        $"{Path.Join(appsPath, desktopName)}.desktop",
        desktopTemplate.Replace("BINARY", binaryPath)
          .Replace("DISPLAY_NAME", $"{game.DisplayName.Replace("Command & Conquer: ", string.Empty)} (NCO)")
          .Replace("/INSTALL_PATH", gamePath)
      );

      eventVisitor.Visit(new ShortcutCreatedEvent(game));
    }
  }

  // TODO: Remove once linux releases use .tar.gz to preserve executable bit
  [SupportedOSPlatform("linux")]
  private void MakeEngineBinariesExecutable(string installRoot)
  {
    foreach (var game in configService.Config.EnabledGames)
    {
      var binaryPath = Path.Join(installRoot, game.InstallPostfix, game.PlatformBinary);

      File.SetUnixFileMode(binaryPath, File.GetUnixFileMode(binaryPath) | UnixFileMode.UserExecute);
    }
  }

  private async Task RunPostInstallConfig(string installRoot, IDownloadEventVisitor eventVisitor)
  {
    // TODO: Deploy basic ini file (if not present) for C&C/RA that sets video mode (fix issue with scaled displays)

    if (OperatingSystem.IsLinux())
    {
      MakeEngineBinariesExecutable(installRoot);
      await GenerateDesktopFiles(installRoot, eventVisitor);
    }

    if (OperatingSystem.IsWindows())
    {
      await InstallMsvcRuntime(eventVisitor);
      await GenerateWindowsShortcuts(installRoot, eventVisitor);
    }
  }

  private void ExtractNcoFileFromZip(
    IReader zipReader,
    string downloadPath,
    GameDataConfig gameConfig,
    IDownloadEventVisitor downloadEventVisitor
  )
  {
    var outputPath = Path.Join(
      downloadPath,
      zipReader.Entry!.Key!.Replace($"{gameConfig.NcoZipPath}/", $"{gameConfig.InstallPostfix}/")
    );
    var outputPathDir = Path.GetDirectoryName(outputPath);

    if (!string.IsNullOrWhiteSpace(outputPathDir))
    {
      Directory.CreateDirectory(outputPathDir);
    }

    zipReader.WriteEntryToFile(outputPath, new ExtractionOptions() { Overwrite = true });
    downloadEventVisitor.Visit(new WriteGameDataFileEvent(zipReader.Entry.Key, outputPath));
  }

  private void ExtractGameFiles(
    IReader zipReader,
    IEntry entry,
    string installRoot,
    IDownloadEventVisitor eventVisitor
  )
  {
    if (entry.IsDirectory)
    {
      return;
    }

    foreach (var dataConfig in configService.Config.EnabledGames)
    {
      if (
        entry.Key?.StartsWith(dataConfig.NcoZipPath, StringComparison.OrdinalIgnoreCase) ?? false
      )
      {
        ExtractNcoFileFromZip(zipReader, installRoot, dataConfig, eventVisitor);
      }
    }
  }

  private async Task<string> GetAssetForNcoRelease()
  {
    var ncoConfig = configService.Config.Nco;
    var ncoRelease = await gitHubClient.Repos[ncoConfig.GitHubRepo.Owner][ncoConfig.GitHubRepo.Name]
      .Releases
      .Tags[ncoConfig.Release]
      .GetAsync() ?? throw new Exception($"Failed to resolve NCO release: {ncoConfig.Release}");

    var osName = OperatingSystem.IsWindows()
      ? "win"
      : (OperatingSystem.IsMacOS() ? "macos" : "linux");

    var osAssetPrefix = ncoConfig.AssetPrefix.Replace("${OS}", osName);

    return ncoRelease.Assets?.Where(a =>
        (a.Name?.StartsWith(osAssetPrefix) ?? false) && !a.Name.Contains("-debug")
      )
      .Select(a => a.BrowserDownloadUrl)
      .FirstOrDefault() ?? throw new Exception($"Failed to resolve NCO zip, release '{ncoConfig.Release}' and OS: {osName}");
  }

  private async Task WithNcoReleaseArchive(
    IDownloadEventVisitor eventVisitor,
    Action<IReader, IEntry> entryHandler
  )
  {
    // fetch release zip url
    eventVisitor.Visit(new FetchNcoReleaseEvent(configService.Config.Nco));

    var assetUrl = await GetAssetForNcoRelease();

    eventVisitor.Visit(new StartNcoReleaseDownloadEvent(assetUrl));

    // request release zip
    using var client = new HttpClient();
    client.Timeout = Timeout.InfiniteTimeSpan;
    using var response = await client.GetAsync(
      assetUrl,
      HttpCompletionOption.ResponseHeadersRead
    );

    response.EnsureSuccessStatusCode();

    // process release zip
    await using var stream = await response.Content.ReadAsStreamAsync();
    using var zipReader = ReaderFactory.OpenReader(stream);

    while (zipReader.MoveToNextEntry())
    {
      entryHandler(zipReader, zipReader.Entry);
    }
  }

  [SupportedOSPlatform("macos")]
  private async Task DownloadMacOs(IDownloadEventVisitor eventVisitor)
  {        
    await WithNcoReleaseArchive(
      eventVisitor,
      (r, e) =>
      {
        if (e.IsDirectory)
        {
          return;
        }

        foreach (var dataConfig in configService.Config.EnabledGames)
        {
          if (!e.Key?.StartsWith(dataConfig.NcoZipPath, StringComparison.OrdinalIgnoreCase) ?? true)
          {
            continue;
          }

          if (string.Equals(Path.GetExtension(e.Key), ".app", StringComparison.OrdinalIgnoreCase))
          {
            // put app bundles on desktop for user
            var outputPath = Path.Join(pathsConfig.UserDesktopPath, Path.GetFileName(e.Key));

            r.WriteEntryToFile(outputPath, new ExtractionOptions() { Overwrite = true });
            eventVisitor.Visit(new WriteGameDataFileEvent(e.Key!, outputPath));

            continue;
          }

          ExtractNcoFileFromZip(r, pathsConfig.AppDataDirectoryPath, dataConfig, eventVisitor);
        }
      }
    );
  }

  public async Task Download(IDownloadEventVisitor eventVisitor)
  {
    try
    {
      if (OperatingSystem.IsMacOS())
      {
        await DownloadMacOs(eventVisitor);
      }
      else
      {
        var installRoot = configService.Config.Nco.PendingInstallPath;

        await WithNcoReleaseArchive(
          eventVisitor,
          (r, e) => ExtractGameFiles(r, e, installRoot, eventVisitor)
        );

        await RunPostInstallConfig(installRoot, eventVisitor);
      }

      eventVisitor.Visit(new FinishNcoReleaseDownloadEvent());
    }
    catch (Exception e)
    {
      eventVisitor.Visit(new DownloadNcoReleaseErrorEvent(e));
    }
  }
}
