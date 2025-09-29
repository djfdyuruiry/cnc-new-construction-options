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

public class GameDataService(LauncherConfigLoader configLoader, Bin2IsoService bin2IsoService, string downloadPath)
{
  private bool _backgroundLoaded;

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
      if (zipReader.Entry.Key != source.Config.File)
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
    var imagePath = Path.Join(downloadPath, source.Config.File);

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
          : Path.Join(installPath, fileList.Key);

        if (!Directory.Exists(outDir))
        {
          Directory.CreateDirectory(outDir);
        }

        foreach (var file in fileList.Value)
        {
          var fileName = file.Split(@"\").Last();

          await GameDiscUtils.ExtractFile(iso, file, Path.Join(outDir, fileName));
        }
      }
    }
    catch (Exception ex)
    {
      Console.Error.WriteLine(ex);
    }
  }

  private async Task DownloadGameData(
    LauncherConfig rootConfig,
    GameDataConfig dataConfig,
    Action<Bitmap> onBackgroundLoaded
  )
  {
    var installPath = Path.Join(downloadPath, dataConfig.InstallPostfix);

    // TODO: Change to create if missing in final version (and no clobber config/save files logic)
    if (Directory.Exists(installPath))
    {
      Directory.Delete(installPath, true);
    }

    Directory.CreateDirectory(installPath);

    var discImages = dataConfig.DiscImagesBySource.First().Value;

    foreach (var imageSource in discImages)
    {
      Console.WriteLine(
        $"Pulling files from disc image '{imageSource.Name}' using provider: {imageSource.Config.Name}"
      );

      await ExtractGameDataFromDiscImage(imageSource, installPath, async iso =>
      {
        if (_backgroundLoaded || !iso.FileExists("setup.bmp"))
        {
          return;
        }

        await using var fileStream = iso.OpenFile(@"setup.bmp", FileMode.Open);

        onBackgroundLoaded(Bitmap.DecodeToHeight(fileStream, 480));

        _backgroundLoaded = true;
      });
    }

    var releaseService = new NcoReleaseService(rootConfig, installPath);

    await releaseService.Download();
  }

  public async Task Download(Action<Bitmap> onBackgroundLoaded)
  {
    var config = configLoader.Load();

    DiscUtils.Complete.SetupHelper.SetupComplete();

    if (!Directory.Exists(downloadPath))
    {
      Directory.CreateDirectory(downloadPath);
    }

    await DownloadGameData(config, config.TiberianDawn, onBackgroundLoaded);
  }
}
