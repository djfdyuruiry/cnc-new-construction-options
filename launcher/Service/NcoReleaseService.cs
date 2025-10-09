using System;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Runtime.Versioning;
using System.Threading;
using System.Threading.Tasks;

using GitHub;
using GitHub.Octokit.Client;
using Microsoft.Kiota.Abstractions.Authentication;
using SharpCompress.Common;
using SharpCompress.Readers;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Model;
using CNC.NCO.Launcher.Model.Events.Download;
using CNC.NCO.Launcher.Util;

namespace CNC.NCO.Launcher.Service;

// TODO: Support macos .app extract and install in Applications (plus data paths outside of app bundle)
public class NcoReleaseService(LauncherConfigService configService, PathsConfig pathsConfig)
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
    }
  }

  /**
   * Windows binaries require MSVC 2022 runtime to run, so we install it for the
   * user here.
   */
  [SupportedOSPlatform("windows")]
  private async Task InstallMsvcRuntime(IDownloadEventVisitor eventVisitor)
  {
    if (configService.Config.NCO.Installed)
    {
      // only needs installed once, so skip on subsequent installer executions
      return;
    }

    eventVisitor.Visit(new FetchMsvcRuntimeEvent());

    var winUtils = new WindowsUtils(pathsConfig);

    await winUtils.InstallMsvcRuntime(configService.Config.NCO.MsvcRuntimeUrl);
  }

  [SupportedOSPlatform("linux")]
  private void MakeEngineBinariesExecutable(string installRoot)
  {
    foreach (var game in configService.Config.EnabledGames)
    {
      var binaryPath = Path.Join(installRoot, game.InstallPostfix, game.PlatformBinary);

      File.SetUnixFileMode(binaryPath, File.GetUnixFileMode(binaryPath) | UnixFileMode.UserExecute);
    }
  }

  [SupportedOSPlatform("linux")]
  private void GenerateDesktopFiles(string installRoot, IDownloadEventVisitor eventVisitor)
  {
    var desktopTemplate = File.ReadAllText(
      Path.Join(pathsConfig.ToolsPath, "nco.desktop")
    );
    var appsPath = Path.Join(pathsConfig.AppDataDirectoryPath, "applications");

    Directory.CreateDirectory(appsPath);

    foreach (var game in configService.Config.EnabledGames)
    {
      var gamePath = Path.Join(installRoot, game.InstallPostfix);
      var binaryPath = Path.Join(gamePath, game.PlatformBinary);
      var desktopName = $"nco-{game.InstallPostfix}";

      File.WriteAllText(
        $"{Path.Join(appsPath, desktopName)}.desktop",
        desktopTemplate.Replace("<BINARY>", binaryPath)
          .Replace("<DISPLAY_NAME>", $"{game.DisplayName.Replace("Command & Conquer:", "C&C -")} (NCO)")
          .Replace("<INSTALL_PATH>", gamePath)
      );
    }
  }

  private async Task RunPostInstallConfig(string installRoot, IDownloadEventVisitor eventVisitor)
  {
    if (OperatingSystem.IsLinux())
    {
      GenerateDesktopFiles(installRoot, eventVisitor);
      MakeEngineBinariesExecutable(installRoot);
    }

    if (OperatingSystem.IsWindows())
    {
      await InstallMsvcRuntime(eventVisitor);  
      await GenerateWindowsShortcuts(installRoot, eventVisitor);
    }

    if (OperatingSystem.IsMacOS())
    {
      // TODO: any required macos config
    }
  }

  private void ExtractNcoFileFromZip(
    GameDataConfig gameConfig,
    IDownloadEventVisitor downloadEventVisitor,
    string downloadPath,
    IReader zipReader)
  {
    var outputPath = Path.Join(
      downloadPath,
      zipReader.Entry!.Key!.Replace($"{gameConfig.NcoZipPath}/", $"{gameConfig.InstallPostfix}/")
    );
    var outputPathDir = Path.GetDirectoryName(outputPath) ?? string.Empty;

    if (!string.IsNullOrWhiteSpace(outputPathDir) && !Directory.Exists(outputPathDir))
    {
      Directory.CreateDirectory(outputPathDir);
    }

    zipReader.WriteEntryToFile(outputPath, new ExtractionOptions() { Overwrite = true });
    downloadEventVisitor.Visit(new WriteGameDataFileEvent(zipReader.Entry.Key, outputPath));
  }

  private void ScanNcoReleaseArchiveFiles(
    string installRoot,
    IDownloadEventVisitor eventVisitor,
    Stream responseStream
  )
  {
    using var zipReader = ReaderFactory.Open(responseStream);

    while (zipReader.MoveToNextEntry())
    {
      if (zipReader.Entry.IsDirectory)
      {
        continue;
      }

      foreach (var dataConfig in configService.Config.EnabledGames)
      {
        if (
          zipReader.Entry.Key?.StartsWith(dataConfig.NcoZipPath, StringComparison.OrdinalIgnoreCase) ?? false
        )
        {
          ExtractNcoFileFromZip(dataConfig, eventVisitor, installRoot, zipReader);
        }
      }
    }
  }

  private async Task<string> GetAssetForNcoRelease()
  {
    using var requestAdapter = RequestAdapter.Create(new AnonymousAuthenticationProvider());
    var githubClient = new GitHubClient(requestAdapter);

    var ncoConfig = configService.Config.NCO;
    var ncoRelease = await githubClient.Repos[ncoConfig.GitHubRepo.Owner][ncoConfig.GitHubRepo.Name]
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
    Action<Stream> responseHandler
  )
  {
    eventVisitor.Visit(new FetchNcoReleaseEvent(configService.Config.NCO));

    var assetUrl = await GetAssetForNcoRelease();

    eventVisitor.Visit(new StartNcoReleaseDownloadEvent(assetUrl));

    using var client = new HttpClient();
    client.Timeout = Timeout.InfiniteTimeSpan;
    using var response = await client.GetAsync(
      assetUrl,
      HttpCompletionOption.ResponseHeadersRead
    );
    
    response.EnsureSuccessStatusCode();

    await using var stream = await response.Content.ReadAsStreamAsync();

    responseHandler(stream);
  }

  public async Task Download(string installRoot, IDownloadEventVisitor eventVisitor)
  {
    try
    {
      // BUG: get 'instance has already started requests' when calling this method for second time
      await WithNcoReleaseArchive(
        eventVisitor,
        s => ScanNcoReleaseArchiveFiles(installRoot, eventVisitor, s)
      );

      await RunPostInstallConfig(installRoot, eventVisitor);

      eventVisitor.Visit(new FinishNcoReleaseDownloadEvent());
    }
    catch (Exception e)
    {
      eventVisitor.Visit(new DownloadNcoReleaseErrorEvent(e));
    }
  }
}
