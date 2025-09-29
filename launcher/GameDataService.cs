using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Threading;
using System.Threading.Tasks;

using Avalonia.Media.Imaging;
using DiscUtils.Iso9660;
using GitHub;
using GitHub.Octokit.Client;
using InstallShieldExtractor;
using Microsoft.Kiota.Abstractions.Authentication;
using SharpCompress.Common;
using SharpCompress.Readers;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Model;

namespace CNC.NCO.Launcher;

public class GameDataService(LauncherConfigLoader configLoader, string cachePath)
{
  private readonly string _cacheOutPath = Path.Join(cachePath, "out");

  private async Task<Stream?> GetStreamForSetupPackageFile(CDReader iso, string name)
  {
    await using var setupStream = iso.OpenFile(@"INSTALL\SETUP.Z", FileMode.Open);
    using var setupPackage = new InstallShieldPackage(setupStream, "SETUP.Z");

    return setupPackage.Contents
      .Where(p => p.EndsWith(name)).Select(p => setupPackage.GetStream(p))
      .FirstOrDefault();
  }

  private async Task<string> ConvertBinToIso(string binPath)
  {
    var isoFileNoExtensions = Path.GetFileNameWithoutExtension(binPath);
    var cuePath = Path.ChangeExtension(binPath, "cue");

    if (File.Exists(cuePath))
    {
      File.Delete(cuePath);
    }

    var convertToCueProc = Process.Start("/usr/bin/bin2iso", [ cuePath, "-c", binPath ]);

    await convertToCueProc.WaitForExitAsync();

    if (convertToCueProc.ExitCode != 0)
    {
      throw new Exception($"Failed to generate cue from bin file: {binPath}");
    }
      
    var convertToIsoProc = Process.Start("/usr/bin/bin2iso", [ cuePath, cachePath, "-t", "1" ]);

    await convertToIsoProc.WaitForExitAsync();

    if (convertToIsoProc.ExitCode != 0)
    {
      throw new Exception($"Failed to extract iso from bin file: {binPath}");
    }

    return Directory.GetFileSystemEntries(Path.GetDirectoryName(cuePath)!)
      .First(f => 
        Path.GetExtension(f).Equals(".iso", StringComparison.InvariantCultureIgnoreCase)
        && Path.GetFileName(f).StartsWith(isoFileNoExtensions, StringComparison.InvariantCultureIgnoreCase)
      );
  }

  private async Task ExtractFile(CDReader iso, string isoOrSetupPath, string outputPath)
  {
    try
    {
      Console.WriteLine($"Extracting {isoOrSetupPath} to {outputPath}");

      await using var sourceFileStream = iso.FileExists(isoOrSetupPath)
        ? iso.OpenFile(isoOrSetupPath, FileMode.Open)
        : (await GetStreamForSetupPackageFile(iso, isoOrSetupPath))!;

      await using var outputStream = File.Open(outputPath, FileMode.Create);
      await sourceFileStream.CopyToAsync(outputStream);
    }
    catch (Exception ex)
    {
      Console.WriteLine($"Error extracting {isoOrSetupPath} to {outputPath}: {ex.Message}");
    }
  }

  private async Task FetchCncIsoIfMissing(DiscImageSource source)
  {
    if (File.Exists(Path.Join(cachePath, source.ImageFileName)))
    {
      return;
    }
    
    Console.WriteLine($"Downloading {source.Url}...");

    using var client = new HttpClient();
    client.Timeout = Timeout.InfiniteTimeSpan;
    using var response = await client.GetAsync(source.Url, HttpCompletionOption.ResponseHeadersRead);

    response.EnsureSuccessStatusCode();
    await using var responseStream = await response.Content.ReadAsStreamAsync();

    if (!source.IsZipped)
    {
      var isoFile = File.Open(Path.Join(cachePath, source.ImageFileName), FileMode.OpenOrCreate, FileAccess.Write);
      await responseStream.CopyToAsync(isoFile);
      
      return;
    }

    using var zipReader = ReaderFactory.Open(responseStream);

    while (zipReader.MoveToNextEntry())
    {
      if (zipReader.Entry.Key == source.ImageFileName)
      {
        Console.WriteLine(zipReader.Entry.Key);
        zipReader.WriteEntryToDirectory(cachePath, new ExtractionOptions() { Overwrite = true });
      }

      break;
    }
  }

  private async Task ExtractFromTdIso(
    DiscImageSource source,
    Func<CDReader, Task> onIsoOpen
  )
  {
    var imagePath = Path.Join(cachePath, source.ImageFileName);

    try
    {
      await FetchCncIsoIfMissing(source);
    }
    catch (Exception ex)
    {
      throw new Exception($"Failed to download CNC ISO {source.ImageFileName} from: {source.Url}", ex);
    }

    Console.WriteLine($"Processing image file: {imagePath}");

    if (Path.GetExtension(imagePath).Equals(".bin", StringComparison.InvariantCultureIgnoreCase))
    {
      try
      {
        imagePath = await ConvertBinToIso(imagePath);
      }
      catch (Exception ex)
      {
        throw new Exception($"Failed to convert bin image to ISO: {source.ImageFileName}", ex);
      }
    }

    try
    {
      await using var isoStream = File.Open(imagePath, FileMode.Open);
      using var iso = new CDReader(isoStream, true);

      await onIsoOpen(iso);

      Console.WriteLine("Extracting files from image");

      foreach (var fileList in source.Files)
      {
        var outDir = fileList.Key == "_root"
          ? _cacheOutPath
          : Path.Join(_cacheOutPath, fileList.Key);

        if (!Directory.Exists(outDir))
        {
          Directory.CreateDirectory(outDir);
        }

        foreach (var file in fileList.Value)
        {
          var fileName = file.Split(@"\").Last();

          await ExtractFile(iso, file, Path.Join(outDir, fileName));
        }
      }
    }
    catch (Exception ex)
    {
      Console.Error.WriteLine(ex);
    }
  }

  public async Task LoadData(Action<Bitmap> onBackgroundLoaded)
  {
    var config = configLoader.Load();

    DiscUtils.Complete.SetupHelper.SetupComplete();

    if (Directory.Exists(_cacheOutPath))
    {
      Directory.Delete(_cacheOutPath, true);
    }

    Directory.CreateDirectory(_cacheOutPath);

    var backgroundLoaded = false;

    foreach (var image in config.TiberianDawn.DiscImages)
    {
      var provider = image.Value.Sources.First();

      Console.WriteLine($"Pulling files from disc image '{image.Key}' using provider: {provider.Name}");

      var source = new DiscImageSource(
        image.Key,
        provider.Url,
        provider.Url.EndsWith(".zip"),
        provider.File,
        image.Value.Provides
      );

      await ExtractFromTdIso(source, async iso =>
      {
        if (backgroundLoaded || !iso.FileExists("setup.bmp"))
        {
          return;
        }

        await using var fileStream = iso.OpenFile(@"setup.bmp", FileMode.Open);

        onBackgroundLoaded(Bitmap.DecodeToHeight(fileStream, 480));

        backgroundLoaded = true;
      });
    }

    try
    {
      var githubClient = new GitHubClient(
        RequestAdapter.Create(new AnonymousAuthenticationProvider())
      );

      var ncoConfig = config.Options;
      var ncoRelease = await githubClient.Repos[ncoConfig.GitHubRepo.Owner][ncoConfig.GitHubRepo.Name]
        .Releases
        .Tags[ncoConfig.Release]
        .GetAsync() ?? throw new Exception($"Failed to resolve NCO release: {ncoConfig.Release}");

      var osName = OperatingSystem.IsWindows()
        ? "win"
        : (OperatingSystem.IsMacOS() ? "macos" : "linux");

      var osAssetPrefix = ncoConfig.AssetPrefix.Replace("${OS}", osName);
      var assetUrl = ncoRelease.Assets?.Where(a =>
          (a.Name?.StartsWith(osAssetPrefix) ?? false) && !a.Name.Contains("-debug")
        )
        .Select(a => a.BrowserDownloadUrl)
        .FirstOrDefault() ?? throw new Exception($"Failed to resolve NCO zip, release '{ncoConfig.Release}' and OS: {osName}");

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
        if (!zipReader.Entry.IsDirectory && (zipReader.Entry.Key?.StartsWith(config.TiberianDawn.NcoZipPath) ?? false))
        {
          var outputPath = Path.Join(_cacheOutPath, zipReader.Entry.Key.Replace($"{config.TiberianDawn.NcoZipPath}/", string.Empty));
          var outputPathDir = Path.GetDirectoryName(outputPath) ?? string.Empty;

          if (!string.IsNullOrEmpty(outputPathDir) && !Directory.Exists(outputPathDir))
          {
            Directory.CreateDirectory(outputPathDir);
          }

          Console.WriteLine($"Deploying NCO file {zipReader.Entry.Key} to: {outputPath}");
          zipReader.WriteEntryToFile(outputPath, new ExtractionOptions() { Overwrite = true });
        }
      }
    }
    catch (Exception e)
    {
      Console.Error.WriteLine(e);
    }
  }

}
