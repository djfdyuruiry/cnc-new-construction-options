using System.Collections.Generic;
using System.Linq;
using Avalonia.Remote.Protocol.Viewport;

namespace CNC.NCO.Launcher.Model;

public class GameDataConfig
{
  public string NcoZipPath { get; set; }
  public string InstallPostfix { get; set; }
  public Dictionary<string, DiscImage> DiscImages { get; set; }
  public Dictionary<string, List<DiscImageSource>> DiscImagesBySource => DiscImages.Values
    .SelectMany(x => x.Sources.Keys)
    .Distinct()
    .ToDictionary(
      p => p,
      p => DiscImages
        .Select(d => d.Value.BuildSources(d.Key).GetValueOrDefault(p))
        .Where(d => d is not null)
        .Select(d => d!)
        .ToList()
    );
}
