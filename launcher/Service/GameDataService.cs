using System;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Threading;
using System.Threading.Tasks;

using Avalonia.Media.Imaging;
using DiscUtils.Iso9660;
using SharpCompress.Common;
using SharpCompress.Readers;

using CNC.NCO.Launcher.Config;
using CNC.NCO.Launcher.Model;
using CNC.NCO.Launcher.Util;

namespace CNC.NCO.Launcher.Service;

public class GameDataService(
  Func<LauncherConfig> configProvider,
  Bin2IsoService bin2IsoService,
  MediaFireDownloadService mediaFireDownloadService,
  PathsConfig paths
)
{
  private void ZipUrlStreamHandler(ZipUrlSpec spec, string installPath, Stream downloadStream)
  {
    using var zipReader = ReaderFactory.Open(downloadStream);
    var fileSuffix = spec.ProvidesFilesEndingWith.ToLower();

    while (zipReader.MoveToNextEntry())
    {
      if (zipReader.Entry.Key?.ToLower().EndsWith(fileSuffix) ?? false)
      {
        Console.WriteLine($"Extracting {zipReader.Entry.Key} to {installPath}/{zipReader.Entry.Key.ToLower()}");

        zipReader.WriteEntryToFile(
          Path.Join(installPath, zipReader.Entry.Key.ToLower()),
          new ExtractionOptions() { Overwrite = true }
        );
      }
    }
  }
  
  private async Task FetchCncIsoIfMissing(DiscImageSource source, string destinationPath)
  {
    if (File.Exists(destinationPath))
    {
      return;
    }
    
    Console.WriteLine($"Downloading {source.Config.Url}...");

    using var client = new HttpClient();
    client.Timeout = Timeout.InfiniteTimeSpan;
    using var response = await client.GetAsync(source.Config.Url, HttpCompletionOption.ResponseHeadersRead);

    response.EnsureSuccessStatusCode();
    await using var responseStream = await response.Content.ReadAsStreamAsync();

    if (!source.Config.IsZipSource)
    {
      var isoFile = File.Open(destinationPath, FileMode.OpenOrCreate, FileAccess.Write);
      await responseStream.CopyToAsync(isoFile);
      
      return;
    }

    using var zipReader = ReaderFactory.Open(responseStream);

    while (zipReader.MoveToNextEntry())
    {
      if (zipReader.Entry.Key == source.Config.File)
      {
        zipReader.WriteEntryToFile(destinationPath, new ExtractionOptions() { Overwrite = true });
      }
    }
  }

  private async Task ExtractGameDataFromDiscImage(
    DiscImageSource source,
    string installPath,
    Func<CDReader, Task> onIsoOpen
  )
  {
    var imagePath = Path.Join(paths.CachePath, source.Config.File);

    try
    {
      await FetchCncIsoIfMissing(source, imagePath);
    }
    catch (Exception ex)
    {
      throw new Exception($"Failed to download CNC ISO {source.Config.File} from: {source.Config.Url}", ex);
    }

    Console.WriteLine($"Processing image file: {imagePath}");

    if (Path.GetExtension(imagePath).Equals(".bin", StringComparison.InvariantCultureIgnoreCase))
    {
      try
      {
        imagePath = await bin2IsoService.ConvertBinToIso(imagePath);
      }
      catch (Exception ex)
      {
        throw new Exception($"Failed to convert bin image to ISO: {source.Config.File}", ex);
      }
    }

    try
    {
      await using var isoStream = File.Open(imagePath, FileMode.Open, FileAccess.Read);
      using var iso = new CDReader(isoStream, true);

      await onIsoOpen(iso);

      Console.WriteLine("Extracting files from image");

      foreach (var fileList in source.Provides)
      {
        var outDir = fileList.Key == "_root"
          ? installPath
          : Path.Join(installPath, fileList.Key.ToLower());

        if (!Directory.Exists(outDir))
        {
          Directory.CreateDirectory(outDir);
        }

        foreach (var file in fileList.Value)
        {
          var fileName = file.Split(@"\").Last();

          await GameDiscUtils.ExtractFile(iso, file, Path.Join(outDir, fileName.ToLower()));
        }
      }
    }
    catch (Exception ex)
    {
      Console.Error.WriteLine(ex);
    }
  }

  private async Task DownloadGameData(
    GameDataConfig dataConfig,
    Action<Bitmap> onSplashScreenLoaded
  )
  {
    var installPath = Path.Join(paths.CachePath, dataConfig.InstallPostfix);

    // TODO: Change to create if missing in final version (and no clobber config/save files logic)
    if (Directory.Exists(installPath))
    {
      Directory.Delete(installPath, true);
    }

    Directory.CreateDirectory(installPath);

    // TODO: Allow user to select source and pass into this method to filter (instead of first)
    var discImages = dataConfig.DiscImagesBySource.First().Value;

    foreach (var imageSource in discImages)
    {
      Console.WriteLine(
        $"Pulling files from disc image '{imageSource.Name}' using provider: {imageSource.Config.Name}"
      );

      await ExtractGameDataFromDiscImage(imageSource, installPath, async iso =>
      {
        if (imageSource.SplashScreenFile is null || !iso.FileExists(imageSource.SplashScreenFile))
        {
          return;
        }

        await using var fileStream = iso.OpenFile(imageSource.SplashScreenFile, FileMode.Open);

        onSplashScreenLoaded(Bitmap.DecodeToHeight(fileStream, 480));
      });
    }

    foreach (var zipUrl in dataConfig.ZipUrls ?? [])
    {
      Console.WriteLine($"Downloading files from ZIP Url: {zipUrl.Url}");

      if (mediaFireDownloadService.IsMediaFireUrl(zipUrl.Url))
      {
        await mediaFireDownloadService.WithFileStream(
          zipUrl.Url, 
          s => ZipUrlStreamHandler(zipUrl, installPath, s)
        );
      }
      else
      {
        using var client = new HttpClient();
        client.Timeout = Timeout.InfiniteTimeSpan;
        using var response = await client.GetAsync(
          zipUrl.Url,
          HttpCompletionOption.ResponseHeadersRead
        );

        response.EnsureSuccessStatusCode();
        await using var responseStream = await response.Content.ReadAsStreamAsync();
        
        ZipUrlStreamHandler(zipUrl, installPath, responseStream);
      }
    }
  }

  public async Task Download(Action<Bitmap> onSplashScreenLoaded)
  {
    var config = configProvider();

    if (!Directory.Exists(paths.CachePath))
    {
      Directory.CreateDirectory(paths.CachePath);
    }

    foreach (var game in new[] { config.TiberianDawn, config.RedAlert })
    {
      await DownloadGameData(game, onSplashScreenLoaded);
    }

    var releaseService = new NcoReleaseService(config, paths.CachePath);
    await releaseService.Download();
  }
}
