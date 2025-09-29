using System.Collections.Generic;
using System.Linq;

namespace CNC.NCO.Launcher.Model;

public class DiscImage
{
  public Dictionary<string, DiscImageSourceConfig> Sources { get; set; }
  public Dictionary<string, List<string>> Provides { get; set; }

  public Dictionary<string, DiscImageSource> BuildSources(string imageName)
  {
    return Sources.ToDictionary(
      s => s.Key,
      s => s.Value.WithName(s.Key)
    ).ToDictionary(
      s => s.Key,
      s => new DiscImageSource
      {
        Name = imageName,
        Config = s.Value,
        Provides = Provides
      }
    );
  }
}
