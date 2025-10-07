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

namespace CNC.NCO.Launcher.Service;

// TODO: Support macos .app extract and install in Applications (plus data paths outside of app bundle)
public class NcoReleaseService(LauncherConfigService configService)
{
  private readonly LauncherConfig _config = configService.Config;

  [SupportedOSPlatform("linux")]
  private void MakeEngineBinariesExecutable(string installRoot)
  {
    foreach (var game in _config.Games.Where(g => g.Enabled))
    {
      var binaryPath = Path.Join(installRoot, game.InstallPostfix, game.PlatformBinary);

      File.SetUnixFileMode(binaryPath, File.GetUnixFileMode(binaryPath) | UnixFileMode.UserExecute);
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

      foreach (var dataConfig in _config.EnabledGames)
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
    var githubClient = new GitHubClient(
      RequestAdapter.Create(new AnonymousAuthenticationProvider())
    );

    var ncoConfig = _config.NCO;
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
    eventVisitor.Visit(new FetchNcoReleaseEvent(_config.NCO));

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
      await WithNcoReleaseArchive(
        eventVisitor,
        s => ScanNcoReleaseArchiveFiles(installRoot, eventVisitor, s)
      );

      if (OperatingSystem.IsLinux())
      {
        MakeEngineBinariesExecutable(installRoot);
      }

      eventVisitor.Visit(new FinishNcoReleaseDownloadEvent());
    }
    catch (Exception e)
    {
      eventVisitor.Visit(new DownloadNcoReleaseErrorEvent(e.Message));
    }
  }
}

