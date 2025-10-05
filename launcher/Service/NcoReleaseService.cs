using System;
using System.IO;
using System.Linq;
using System.Net.Http;
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

public class NcoReleaseService(LauncherConfigService configService, PathsConfig pathsConfig)
{
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

    if (!string.IsNullOrEmpty(outputPathDir) && !Directory.Exists(outputPathDir))
    {
      Directory.CreateDirectory(outputPathDir);
    }

    zipReader.WriteEntryToFile(outputPath, new ExtractionOptions() { Overwrite = true });
    downloadEventVisitor.Visit(new WriteGameDataFileEvent(zipReader.Entry.Key, outputPath));
  }

  private async Task<string> GetAssetForNcoRelease()
  {
    var githubClient = new GitHubClient(
      RequestAdapter.Create(new AnonymousAuthenticationProvider())
    );

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

  public async Task Download(IDownloadEventVisitor eventVisitor)
  {
    try
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
      await using var responseStream = await response.Content.ReadAsStreamAsync();

      using var zipReader = ReaderFactory.Open(responseStream);

      while (zipReader.MoveToNextEntry())
      {
        if (zipReader.Entry.IsDirectory)
        {
          continue;
        }

        foreach (var dataConfig in configService.Config.Games)
        {
          if (
            zipReader.Entry.Key?.StartsWith(dataConfig.NcoZipPath, StringComparison.OrdinalIgnoreCase) ?? false
          )
          {
            ExtractNcoFileFromZip(dataConfig, eventVisitor, configService.Config.NCO.InstallPath!, zipReader);
          }
        }
      }
    }
    catch (Exception e)
    {
      eventVisitor.Visit(new DownloadNcoReleaseErrorEvent(e.Message));
    }
  }
}

