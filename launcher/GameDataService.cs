using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Threading;
using System.Threading.Tasks;

using Avalonia.Media.Imaging;
using DiscUtils.Iso9660;
using InstallShieldExtractor;
using SharpCompress.Common;
using SharpCompress.Readers;

namespace CNC.NCO.Launcher;

public class GameDataService
{
  private static async Task<Stream?> GetStreamForSetupPackageFile(CDReader iso, string name)
  {
    await using var setupStream = iso.OpenFile(@"INSTALL\SETUP.Z", FileMode.Open);
    using var setupPackage = new InstallShieldPackage(setupStream, "SETUP.Z");

    return setupPackage.Contents
      .Where(p => p.EndsWith(name)).Select(p => setupPackage.GetStream(p))
      .FirstOrDefault();
  }

  private static async Task<string> ConvertBinToIso(string cachePath, string binPath)
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

  private static async Task ExtractFile(CDReader iso, string cacheOutPath, string fullFile, string? stripPrefix = null, string? outFile = null)
  {
    var resolvedStripPrefix = stripPrefix ?? string.Empty;
    var resolvedOutFile = outFile ?? fullFile;
    var outputPath = Path.Join(
      cacheOutPath,
      !string.IsNullOrEmpty(resolvedStripPrefix) && resolvedOutFile.StartsWith(resolvedStripPrefix)
        ? resolvedOutFile.Replace(resolvedStripPrefix, string.Empty)
        : resolvedOutFile
    );

    if (File.Exists(outputPath))
    {
      Console.WriteLine($"Skipping already extracted file: {outputPath}");
      return;
    }

    try
    {
      Console.WriteLine($"Extracting {fullFile} to {outputPath}");

      await using var sourceFileStream = iso.FileExists(fullFile)
        ? iso.OpenFile(fullFile, FileMode.Open)
        : (await GetStreamForSetupPackageFile(iso, fullFile))!;

      await using var outputStream = File.Open(outputPath, FileMode.Create);
      await sourceFileStream.CopyToAsync(outputStream);
    }
    catch (Exception ex)
    {
      Console.WriteLine($"Error extracting {fullFile} to {outputPath}: {ex.Message}");
    }
  }

  private static async Task FetchCncIsoIfMissing(DiscImageSource source, string cachePath)
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

  private static async Task ExtractFromTdIso(
    DiscImageSource source,
    string cachePath,
    string cacheOutPath,
    Func<CDReader, Task> onIsoOpen
  )
  {
    var imagePath = Path.Join(cachePath, source.ImageFileName);

    await FetchCncIsoIfMissing(
      source,
      cachePath
    );

    Console.WriteLine($"Processing image file: {imagePath}");

    if (Path.GetExtension(imagePath).Equals(".bin", StringComparison.InvariantCultureIgnoreCase))
    {
      imagePath = await ConvertBinToIso(cachePath, imagePath);
    }

    try
    {
      await using var isoStream = File.Open(imagePath, FileMode.Open);
      using var iso = new CDReader(isoStream, true);

      await onIsoOpen(iso);

      Console.WriteLine("Extracting files from image");

      foreach (var file in source.Files)
      {
        await ExtractFile(iso, cacheOutPath, file, source.StripFilePrefix);
      }

      var factionFiles = new[]
      {
        "GENERAL.MIX",
        "MOVIES.MIX",
        "SCORES.MIX"
      };

      var factionCachePath = Path.Join(cacheOutPath, source.Faction);

      Console.WriteLine("Extracting faction files from image");

      if (!Directory.Exists(factionCachePath))
      {
        Directory.CreateDirectory(factionCachePath);
      }

      foreach (var file in factionFiles)
      {
        await ExtractFile(iso, cacheOutPath, file, source.StripFilePrefix, outFile: Path.Join(source.Faction, file));
      }
    }
    catch (Exception ex)
    {
      Console.Error.WriteLine(ex);
    }
  }

  public async Task LoadData(Action<Bitmap> onBackgroundLoaded)
  {
    DiscUtils.Complete.SetupHelper.SetupComplete();

    var cachePath = Path.Join(AppContext.BaseDirectory, ".cache");
    var cacheOutPath = Path.Join(cachePath, "out");

    if (Directory.Exists(cacheOutPath))
    {
      Directory.Delete(cacheOutPath, true);
    }

    Directory.CreateDirectory(cacheOutPath);

    var backgroundLoaded = false;
    var baseUrl = "https://bigdownloads.cnc-comm.com/cnc1";
    var baseGameFiles = new[]
    {
      "CCLOCAL.MIX",
      "CONQUER.MIX",
      "DESEICNH.MIX",
      "DESERT.MIX",
      "SOUNDS.MIX",
      "SPEECH.MIX",
      "TEMPERAT.MIX",
      "TEMPICNH.MIX",
      "TRANSIT.MIX",
      "UPDATE.MIX",
      "UPDATEC.MIX",
      "WINTER.MIX",
      "WINTICNH.MIX"
    };

    var sources = new DiscImageSource[]
    {
      new ("GDI", $"{baseUrl}/CNC95_GDI.zip", true, "CNC95_GDI.iso", baseGameFiles),
      new ("NOD", $"{baseUrl}/CNC95_Nod.zip",  true, "CNC95_Nod.iso", baseGameFiles),
      new ("COVERTOPS", $"{baseUrl}/CNC_Covertops.zip", true, "CD3_Covertops.bin",
        [ @"INSTALL\SC-000.MIX", @"INSTALL\SC-001.MIX", @"INSTALL\SPEECH.MIX" ],
        @"INSTALL\"
      )
    };

    foreach (var source in sources)
    {
      await ExtractFromTdIso(source, cachePath, cacheOutPath, async iso =>
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
      using var client = new HttpClient { Timeout = Timeout.InfiniteTimeSpan };
      using var response = await client.GetAsync(
        "https://github.com/djfdyuruiry/cnc-new-construction-options/releases/download/latest/vanilla-conquer-nco-linux-clang-ubuntu-24.04-x86_64-c642166.zip",
        HttpCompletionOption.ResponseHeadersRead
      );
    
      response.EnsureSuccessStatusCode();
      await using var responseStream = await response.Content.ReadAsStreamAsync();

      using var zipReader = ReaderFactory.Open(responseStream);

      while (zipReader.MoveToNextEntry())
      {
        if (!zipReader.Entry.IsDirectory && (zipReader.Entry.Key?.StartsWith("td") ?? false))
        {
          var outputPath = Path.Join(cacheOutPath, zipReader.Entry.Key.Replace("td/", string.Empty));
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