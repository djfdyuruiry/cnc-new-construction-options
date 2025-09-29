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

using CNC.NCO.Launcher.Model;

namespace CNC.NCO.Launcher.Service;

public class NcoReleaseService(LauncherConfig config, string downloadPath)
{
  private void ExtractNcoFileFromZip(GameDataConfig gameConfig, IReader zipReader)
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

    Console.WriteLine($"Deploying NCO file {zipReader.Entry.Key} to: {outputPath}");
    zipReader.WriteEntryToFile(outputPath, new ExtractionOptions() { Overwrite = true });
  }

  private async Task<string> GetAssetForNcoRelease()
  {
    var githubClient = new GitHubClient(
      RequestAdapter.Create(new AnonymousAuthenticationProvider())
    );

    var ncoConfig = config.NCO;
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

  public async Task Download()
  {
    try
    {
      var assetUrl = await GetAssetForNcoRelease();

      Console.WriteLine($"Downloading NCO release from URL: {assetUrl}");

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
        foreach (var dataConfig in new[] { config.TiberianDawn, config.RedAlert})
        {
          if (
            zipReader.Entry.IsDirectory || (!(zipReader.Entry.Key?.StartsWith(dataConfig.NcoZipPath) ?? false))
          )
          {
            continue;
          }

          ExtractNcoFileFromZip(dataConfig, zipReader);
        }
      }
    }
    catch (Exception e)
    {
      Console.Error.WriteLine(e);
    }
  }
}
