using System.Collections.Generic;
using System.Linq;

namespace CNC.NCO.Launcher.Model;

public class DiscImage
{
  public string? DisplayName { get; set; }
  public Dictionary<string, DiscImageSourceConfig> Sources { get; set; }
  public string? SplashScreenFile { get; set; }
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
        DisplayName = DisplayName ?? imageName,
        Config = s.Value,
        SplashScreenFile = SplashScreenFile,
        Provides = Provides
      }
    );
  }
}
