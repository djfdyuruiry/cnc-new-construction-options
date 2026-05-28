using System;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using System.Threading;
using System.Threading.Tasks;

using RestSharp;
using SharpCompress.Common;
using SharpCompress.Readers;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Model;
using CNC.NCO.Launcher.Model.Events.Download;
using CNC.NCO.Launcher.Util;

namespace CNC.NCO.Launcher.Service;

public class NcoReleaseService(LauncherConfigService configService, IRestClient githubClient, PathsConfig pathsConfig)
{
  [SupportedOSPlatform("windows")]
  private async Task GenerateWindowsShortcuts(string installRoot, IDownloadEventVisitor eventVisitor)
  {
    var winUtils = new WindowsUtils(pathsConfig);

    // game engine shortcuts
    foreach (var game in configService.Config.EnabledGames)
    {
      var gameBinaryPath = $"{Path.Join(installRoot, game.InstallPostfix, game.Binary)}.exe";

      await winUtils.CreateShortcut(
        $"{game.DisplayName.Replace("Command & Conquer:", "C&C -")} (NCO)",
        gameBinaryPath
      );

      eventVisitor.Visit(new ShortcutCreatedEvent(game.DisplayName));
    }
  
    if (configService.Config.Nco.Installed)
    {
      // don't reinstall the launcher (we might be running a previously installed launcher now)
      return;
    }

    // launcher shortcut
    var launcherBinary = configService.Config.Nco.LauncherBinary;
    var launcherBinaryPath = $"{Path.Join(installRoot, LauncherConfig.LauncherDirectory, launcherBinary)}.exe";

    await winUtils.CreateShortcut("NCO Launcher", launcherBinaryPath);

    eventVisitor.Visit(new ShortcutCreatedEvent("NCO Launcher"));
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
    var engineTemplate = await File.ReadAllTextAsync(
      Path.Join(pathsConfig.ToolsPath, "nco.desktop")
    );
    var appsPath = Path.Join(pathsConfig.AppDataDirectoryPath, "applications");

    Directory.CreateDirectory(appsPath);

    // game engine shortcuts
    foreach (var game in configService.Config.EnabledGames)
    {
      var gamePath = Path.Join(installRoot, game.InstallPostfix);
      var binaryPath = Path.Join(gamePath, game.PlatformBinary);
      var desktopName = $"nco-{game.InstallPostfix}";

      await File.WriteAllTextAsync(
        $"{Path.Join(appsPath, desktopName)}.desktop",
        engineTemplate.Replace("BINARY", binaryPath)
          .Replace("DISPLAY_NAME", $"{game.DisplayName.Replace("Command & Conquer: ", string.Empty)} (NCO)")
          .Replace("/INSTALL_PATH", gamePath)
      );

      eventVisitor.Visit(new ShortcutCreatedEvent(game.DisplayName));
    }

    if (configService.Config.Nco.Installed)
    {
      // don't reinstall launcher files (we might be running a previously installed launcher now)
      return;
    }

    // launcher shortcut
    var launcherTemplate = await File.ReadAllTextAsync(
      Path.Join(pathsConfig.ToolsPath, "nco-launcher.desktop")
    );

    var launcherPath = Path.Join(installRoot, LauncherConfig.LauncherDirectory);

    await File.WriteAllTextAsync(
      $"{Path.Join(appsPath, "nco-launcher")}.desktop",
      launcherTemplate.Replace("BINARY", $"'{Path.Join(launcherPath, configService.Config.Nco.LauncherBinary)}'")
        .Replace("/INSTALL_PATH", $"{launcherPath}")
    );

    eventVisitor.Visit(new ShortcutCreatedEvent("NCO Launcher"));
  }

  [SupportedOSPlatform("linux")]
  private void MakeEngineBinariesExecutable(string installRoot)
  {
    // game engine binaries
    foreach (var game in configService.Config.EnabledGames)
    {
      var binaryPath = Path.Join(installRoot, game.InstallPostfix, game.PlatformBinary);

      File.SetUnixFileMode(binaryPath, File.GetUnixFileMode(binaryPath) | UnixFileMode.UserExecute);
    }

    if (configService.Config.Nco.Installed)
    {
      // don't reconfigure the launcher (we might be running a previously installed launcher now)
      return;
    }

    // launcher binary
    var launcherBinary = configService.Config.Nco.LauncherBinary;
    var launcherPath = Path.Join(installRoot, LauncherConfig.LauncherDirectory, launcherBinary);

    File.SetUnixFileMode(launcherPath, File.GetUnixFileMode(launcherPath) | UnixFileMode.UserExecute);
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
    Func<string, string> pathTransformer,
    IDownloadEventVisitor downloadEventVisitor
  )
  {
    var entryFile = zipReader.Entry.Key!;
    var outputPath = Path.Join(
      downloadPath,
      pathTransformer(entryFile)
    );
    var outputPathDir = Path.GetDirectoryName(outputPath);

    if (!string.IsNullOrWhiteSpace(outputPathDir))
    {
      Directory.CreateDirectory(outputPathDir);
    }

    zipReader.WriteEntryToFile(outputPath, new ExtractionOptions() { Overwrite = true });
    downloadEventVisitor.Visit(new WriteGameDataFileEvent(entryFile, outputPath));
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

    foreach (var gameConfig in configService.Config.EnabledGames)
    {
      if (
        entry.Key?.StartsWith(gameConfig.NcoZipPath, StringComparison.OrdinalIgnoreCase) ?? false
      )
      {
        ExtractNcoFileFromZip(
          zipReader,
          installRoot,
          p => p.Replace($"{gameConfig.NcoZipPath}/", $"{gameConfig.InstallPostfix}/"),
          eventVisitor
        );
      }
    }
  }

  private async Task<string> GetAssetUrlForNcoRelease(IDownloadEventVisitor eventVisitor, string assetPrefix)
  {
    eventVisitor.Visit(new FetchNcoReleaseEvent(configService.Config.Nco));
    
    var ncoConfig = configService.Config.Nco;
    var ncoRelease = await githubClient.ExecuteGetAsync<GitHubApiRelease>(
      $"/repos/{ncoConfig.GitHubRepo.Owner}/{ncoConfig.GitHubRepo.Name}/releases/tags/{ncoConfig.Release}"
    );

    if (ncoRelease.StatusCode != HttpStatusCode.OK || ncoRelease.Data is null)
    {
      throw new Exception($"Failed to resolve NCO release: {ncoConfig.Release}", ncoRelease.ErrorException);
    }

    var osName = OperatingSystem.IsWindows()
      ? "win"
      : (OperatingSystem.IsMacOS() ? "macos" : "linux");

    if (assetPrefix.Contains("launcher", StringComparison.InvariantCultureIgnoreCase) && OperatingSystem.IsMacOS())
    {
      // separate launcher archives for different macOS processor types
      var arch = RuntimeInformation.OSArchitecture == Architecture.Arm64 ? "arm64" : "x64";
      osName = $"{osName}-{arch}";
    }

    var osAssetPrefix = assetPrefix.Replace("${OS}", osName);

    return ncoRelease.Data.Assets?.Where(a =>
        (a.Name?.StartsWith(osAssetPrefix) ?? false) && !a.Name.Contains("-debug")
      )
      .Select(a => a.BrowserDownloadUrl)
      .FirstOrDefault() ?? throw new Exception($"Failed to resolve NCO zip, release '{ncoConfig.Release}' and OS: {osName}");
  }

  private void ExtractLauncherFiles(
    IReader zipReader,
    IEntry entry,
    string installRoot,
    IDownloadEventVisitor downloadEventVisitor
  )
  {
    if (entry.IsDirectory)
    {
      return;
    }

    ExtractNcoFileFromZip(
      zipReader,
      installRoot,
      p => Path.Join(LauncherConfig.LauncherDirectory, p),
      downloadEventVisitor
    );
  }

  private async Task DownloadLauncher(IDownloadEventVisitor eventVisitor)
  {
    try
    {
      var installRoot = configService.Config.Nco.PendingInstallPath;
      await WithNcoArchive(
        eventVisitor,
        configService.Config.Nco.LauncherAssetPrefix,
        url => eventVisitor.Visit(new StartNcoLauncherDownloadEvent(url)),
        (r, e) => ExtractLauncherFiles(r, e, installRoot, eventVisitor)
      );

      eventVisitor.Visit(new FinishNcoLauncherDownloadEvent());
    }
    catch (Exception e)
    {
      eventVisitor.Visit(new DownloadNcoLauncherErrorEvent(e));
    }
  }

  [SupportedOSPlatform("macos")]
  private async Task DownloadMacOsLauncher(IDownloadEventVisitor eventVisitor)
  {
    await WithNcoArchive(
      eventVisitor,
      configService.Config.Nco.LauncherAssetPrefix,
      url => eventVisitor.Visit(new StartNcoLauncherDownloadEvent(url)),
      (r, e) =>
      {
        if (e.IsDirectory || !string.Equals(Path.GetExtension(e.Key), ".app", StringComparison.OrdinalIgnoreCase))
        {
          return;
        }

        // put app bundle on desktop for user
        var outputPath = Path.Join(pathsConfig.UserDesktopPath, Path.GetFileName(e.Key));

        r.WriteEntryToFile(outputPath, new ExtractionOptions() { Overwrite = true });
        eventVisitor.Visit(new WriteGameDataFileEvent(e.Key!, outputPath));
      }
    );
  }

  private async Task WithNcoArchive(
    IDownloadEventVisitor eventVisitor,
    string assetPrefix,
    Action<string> downloadStartCallback,
    Action<IReader, IEntry> entryHandler
  )
  {
    // fetch archive zip url
    var assetUrl = await GetAssetUrlForNcoRelease(eventVisitor, assetPrefix);

    downloadStartCallback(assetUrl);

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
  private async Task DownloadGameEngineMacOs(IDownloadEventVisitor eventVisitor)
  {        
    await WithNcoArchive(
      eventVisitor,
      configService.Config.Nco.GameBinaryAssetPrefix,
      url => eventVisitor.Visit(new StartNcoReleaseDownloadEvent(url)),
      (r, e) =>
      {
        if (e.IsDirectory)
        {
          return;
        }

        foreach (var gameConfig in configService.Config.EnabledGames)
        {
          if (!e.Key?.StartsWith(gameConfig.NcoZipPath, StringComparison.OrdinalIgnoreCase) ?? true)
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

          ExtractNcoFileFromZip(
            r,
            pathsConfig.AppDataDirectoryPath, 
            p => p.Replace($"{gameConfig.NcoZipPath}/", $"{gameConfig.InstallPostfix}/"),
            eventVisitor
          );
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
        await DownloadGameEngineMacOs(eventVisitor);
        eventVisitor.Visit(new FinishNcoReleaseDownloadEvent());

        if (!configService.Config.Nco.Installed)
        {
          // don't reinstall the launcher (we might be running a previously installed launcher now)
          await DownloadMacOsLauncher(eventVisitor);
        }

        return;
      }

      var installRoot = configService.Config.Nco.PendingInstallPath;

      await WithNcoArchive(
        eventVisitor,
        configService.Config.Nco.GameBinaryAssetPrefix,
        url => eventVisitor.Visit(new StartNcoReleaseDownloadEvent(url)),
        (r, e) => ExtractGameFiles(r, e, installRoot, eventVisitor)
      );

      await RunPostInstallConfig(installRoot, eventVisitor);

      eventVisitor.Visit(new FinishNcoReleaseDownloadEvent());

      if (!configService.Config.Nco.Installed)
      {
        // don't reinstall the launcher (we might be running a previously installed launcher now)
        await DownloadLauncher(eventVisitor);
      }
    }
    catch (Exception e)
    {
      eventVisitor.Visit(new DownloadNcoReleaseErrorEvent(e));
    }
  }

}
