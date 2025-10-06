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
      if (!(zipReader.Entry.Key?.EndsWith(fileSuffix, StringComparison.OrdinalIgnoreCase) ?? false))
      {
        continue;
      }

      Console.WriteLine($"Extracting {zipReader.Entry.Key} to {installPath}/{zipReader.Entry.Key.ToLower()}");

      var destPath = Path.Join(installPath, zipReader.Entry.Key.ToLower());

      zipReader.WriteEntryToFile(destPath, new ExtractionOptions() { Overwrite = true });

      downloadEventVisitor.Visit(new WriteGameDataFileEvent(zipReader.Entry.Key, destPath));
    }
  }

  private async Task DownloadGameZipUrlFiles(
    ZipUrlSpec zip,
    IDownloadEventVisitor downloadEventVisitor,
    string installPath
  )
  {
    Console.WriteLine($"Downloading files from ZIP Url: {zip.Url}");

    downloadEventVisitor.Visit(new StartZipUrlDownloadEvent(zip));

    if (mediaFireDownloadService.IsMediaFireUrl(zip.Url))
    {
      await mediaFireDownloadService.WithFileStream(
        zip.Url,
        s => ZipUrlStreamHandler(zip, downloadEventVisitor, installPath, s)
      );

      return;
    }

    using var client = new HttpClient();
    client.Timeout = Timeout.InfiniteTimeSpan;
    using var response = await client.GetAsync(
      zip.Url,
      HttpCompletionOption.ResponseHeadersRead
    );

    response.EnsureSuccessStatusCode();
    await using var responseStream = await response.Content.ReadAsStreamAsync();

    ZipUrlStreamHandler(zip, downloadEventVisitor, installPath, responseStream);

    downloadEventVisitor.Visit(new FinishZipUrlDownloadEvent(zip));
  }

  private async Task ScanDiscImageFiles(
    CDReader iso,
    DiscImageSource source,
    IDownloadEventVisitor downloadEventVisitor,
    string installPath
  )
  {
    Console.WriteLine("Extracting files from image");

    foreach (var fileList in source.Provides)
    {
      var outDir = fileList.Key.Equals(DiscImage.RootPlaceholder, StringComparison.OrdinalIgnoreCase)
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

        await GameDiscUtils.ExtractFile(iso, downloadEventVisitor, file, destPath);
      }
    }
  }

  private async Task<bool> FetchCncIsoIfMissing(
    DiscImageSource source,
    string destinationPath,
    IDownloadEventVisitor downloadEventVisitor
  )
  {
    if (File.Exists(destinationPath))
    {
      return false;
    }

    downloadEventVisitor.Visit(new StartDiscImageDownloadEvent(source));

    using var client = new HttpClient();
    client.Timeout = Timeout.InfiniteTimeSpan;
    using var response = await client.GetAsync(source.Config.Url, HttpCompletionOption.ResponseHeadersRead);

    response.EnsureSuccessStatusCode();
    await using var responseStream = await response.Content.ReadAsStreamAsync();

    if (!source.Config.IsZipSource)
    {
      await using var isoFile = File.Open(destinationPath, FileMode.OpenOrCreate, FileAccess.Write);
      await responseStream.CopyToAsync(isoFile);

      return true;
    }

    using var zipReader = ReaderFactory.Open(responseStream);

    while (zipReader.MoveToNextEntry())
    {
      if (zipReader.Entry.Key == source.Config.File)
      {
        zipReader.WriteEntryToFile(destinationPath, new ExtractionOptions() { Overwrite = true });
      }
    }

    return !File.Exists(destinationPath)
      ? throw new FileNotFoundException("Could not locate image file inside ZIP archive", source.Config.File)
      : true;
  }

  private async Task ExtractGameDataFromDiscImage(
    DiscImageSource source,
    IDownloadEventVisitor downloadEventVisitor,
    string installPath,
    Func<CDReader, Task> onIsoOpen)
  {
    var imagePath = Path.Join(paths.CachePath, source.Config.File);

    // fetch ISO image (if required)
    try
    {
      if (await FetchCncIsoIfMissing(source, imagePath, downloadEventVisitor))
      {
        downloadEventVisitor.Visit(new FinishDiscImageDownloadEvent(source));
      }
    }
    catch (Exception ex)
    {
      throw new GameDataDownloadException(
        $"Failed to fetch CNC ISO '{source.Config.File}' from: {source.Config.Url}",
        ex
      );
    }

    // covert .bin to .iso (if required)
    if (Path.GetExtension(imagePath).Equals(".bin", StringComparison.OrdinalIgnoreCase))
    {
      try
      {
        downloadEventVisitor.Visit(new ConvertDiscImageEvent(source, "bin", "iso"));
        imagePath = await bin2IsoService.ConvertBinToIso(imagePath);
      }
      catch (Exception ex)
      {
        throw new GameDataDownloadException($"Failed to convert bin image to ISO: {source.Config.File}", ex);
      }
    }

    // open ISO image and extract files
    try
    {
      await using var isoStream = File.Open(imagePath, FileMode.Open, FileAccess.Read);
      using var iso = new CDReader(isoStream, true);

      downloadEventVisitor.Visit(new StartDiscImageFileScanEvent(source));

      await onIsoOpen(iso);

      await ScanDiscImageFiles(iso, source, downloadEventVisitor, installPath);

      downloadEventVisitor.Visit(new FinishDiscImageFileScanEvent(source));
    }
    catch (Exception ex)
    {
      throw new GameDataDownloadException($"Failed to open ISO disc image: {source.DisplayName ?? source.Name}", ex);
    }
  }

  private async Task DownloadGameDiscImageFiles(
    GameDataConfig dataConfig,
    IDownloadEventVisitor downloadEventVisitor,
    Action<Bitmap> onSplashScreenLoaded,
    string installPath
  )
  {
    // TODO: Allow user to select source and pass into this method to filter (instead of first)
    var discImages = dataConfig.DiscImagesBySource.First().Value;

    foreach (var imageSource in discImages.Where(d => d.Enabled).OrderBy(d => d.SortOrder))
    {
      await ExtractGameDataFromDiscImage(
        imageSource,
        downloadEventVisitor,
        installPath,
        i => OnIsoOpen(imageSource, i)
      );
    }

    return;

    async Task OnIsoOpen(DiscImageSource imageSource, CDReader iso)
    {
      if (imageSource.SplashScreenFile is not null && iso.FileExists(imageSource.SplashScreenFile))
      {
        await using var fileStream = iso.OpenFile(imageSource.SplashScreenFile, FileMode.Open);
        onSplashScreenLoaded(Bitmap.DecodeToHeight(fileStream, 480));
      }
    }
  }

  public async Task Download(IDownloadEventVisitor downloadEventVisitor, Action<Bitmap> onSplashScreenLoaded)
  {
    GameDataConfig? currentGame = null;

    try
    {
      DirectoryUtils.CreateDirectoryIfMissing(paths.CachePath);

      foreach (var game in _config.Games.Where(c => c.Enabled).OrderBy(c => c.SortOrder))
      {
        currentGame  = game;

        var installPath = Path.Join(_config.NCO.InstallPath, game.InstallPostfix);

        DirectoryUtils.CreateDirectoryIfMissing(installPath);

        downloadEventVisitor.Visit(new StartDownloadGameDataEvent(game));

        await DownloadGameDiscImageFiles(game, downloadEventVisitor, onSplashScreenLoaded, installPath);

        foreach (var zipUrl in (game.ZipUrls ?? []).Where(z => z.Enabled).OrderBy(z => z.SortOrder))
        {
          await DownloadGameZipUrlFiles(zipUrl, downloadEventVisitor, installPath);
        }

        downloadEventVisitor.Visit(new FinishDownloadGameDataEvent(game));
      }
    }
    catch (Exception ex)
    {
      downloadEventVisitor.Visit(new DownloadGameDataErrorEvent(currentGame, ex.Message));
    }
 }

}

