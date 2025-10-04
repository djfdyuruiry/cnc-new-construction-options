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
using CNC.NCO.Launcher.Model.Events.Download;
using CNC.NCO.Launcher.Util;

namespace CNC.NCO.Launcher.Service;

public class GameDataService(
  LauncherConfigService configService,
  Bin2IsoService bin2IsoService,
  MediaFireDownloadService mediaFireDownloadService,
  PathsConfig paths
)
{
  private readonly LauncherConfig _config = configService.Config;

  private void ZipUrlStreamHandler(
    ZipUrlSpec spec,
    IDownloadEventVisitor downloadEventVisitor,
    string installPath,
    Stream downloadStream
  )
  {
    using var zipReader = ReaderFactory.Open(downloadStream);
    var fileSuffix = spec.ProvidesFilesEndingWith.ToLower();

    while (zipReader.MoveToNextEntry())
    {
      if (zipReader.Entry.Key?.ToLower().EndsWith(fileSuffix) ?? false)
      {
        Console.WriteLine($"Extracting {zipReader.Entry.Key} to {installPath}/{zipReader.Entry.Key.ToLower()}");

        var destPath = Path.Join(installPath, zipReader.Entry.Key.ToLower());

        zipReader.WriteEntryToFile(
          destPath,
          new ExtractionOptions() { Overwrite = true }
        );
        
        downloadEventVisitor.Visit(new WriteGameDataFileEvent(zipReader.Entry.Key, destPath));
      }
    }
  }
  
  private async Task FetchCncIsoIfMissing(
    DiscImageSource source,
    string destinationPath,
    IDownloadEventVisitor downloadEventVisitor
  )
  {
    if (File.Exists(destinationPath))
    {
      return;
    }
    
    downloadEventVisitor.Visit(new StartDiscImageDownloadEvent(source));
  
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

  private async Task ExtractGameDataFromDiscImage(DiscImageSource source,
    IDownloadEventVisitor downloadEventVisitor,
    string installPath,
    Func<CDReader, Task> onIsoOpen)
  {
    var imagePath = Path.Join(paths.CachePath, source.Config.File);

    try
    {
      await FetchCncIsoIfMissing(source, imagePath, downloadEventVisitor);
    }
    catch (Exception ex)
    {
      throw new Exception($"Failed to download CNC ISO {source.Config.File} from: {source.Config.Url}", ex);
    }

    if (Path.GetExtension(imagePath).Equals(".bin", StringComparison.InvariantCultureIgnoreCase))
    {
      try
      {
        downloadEventVisitor.Visit(new ConvertDiscImageEvent(source, "bin", "iso"));
        imagePath = await bin2IsoService.ConvertBinToIso(imagePath);
      }
      catch (Exception ex)
      {
        throw new Exception($"Failed to convert bin image to ISO: {source.Config.File}", ex);
      }
    }
  
    await using var isoStream = File.Open(imagePath, FileMode.Open, FileAccess.Read);
    using var iso = new CDReader(isoStream, true);

    downloadEventVisitor.Visit(new StartDiscImageFileScanEvent(source));

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
        var destPath = Path.Join(outDir, fileName.ToLower());

        await GameDiscUtils.ExtractFile(iso, file, destPath);
        
        downloadEventVisitor.Visit(new WriteGameDataFileEvent(file, destPath));
      }
    }
  }

  private async Task DownloadGameData(GameDataConfig dataConfig,
    IDownloadEventVisitor downloadEventVisitor,
    Action<Bitmap> onSplashScreenLoaded)
  {
    var installPath = Path.Join(_config.NCO.InstallPath, dataConfig.InstallPostfix);

    if (!Directory.Exists(installPath))
    {
      Directory.CreateDirectory(installPath);
    }
  
    downloadEventVisitor.Visit(new StartDownloadGameDataEvent(dataConfig));

    // TODO: Allow user to select source and pass into this method to filter (instead of first)
    var discImages = dataConfig.DiscImagesBySource.First().Value;

    foreach (var imageSource in discImages)
    {
      await ExtractGameDataFromDiscImage(imageSource, downloadEventVisitor, installPath, async iso =>
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
          s => ZipUrlStreamHandler(zipUrl, downloadEventVisitor, installPath, s)
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
        
        ZipUrlStreamHandler(zipUrl, downloadEventVisitor, installPath, responseStream);
      }
    }
  }

  public async Task Download(IDownloadEventVisitor downloadEventVisitor, Action<Bitmap> onSplashScreenLoaded)
  {
    if (!Directory.Exists(paths.CachePath))
    {
      Directory.CreateDirectory(paths.CachePath);
    }

    foreach (var game in new[] { _config.TiberianDawn, _config.RedAlert })
    {
      if (!game.Enabled)
      {
        Console.WriteLine($"Skipping disabled game: {game.DisplayName}");
        continue;
      }

      await DownloadGameData(game, downloadEventVisitor, onSplashScreenLoaded);
    }
  }
}

