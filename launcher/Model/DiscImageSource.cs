using System.Collections.Generic;
using YamlDotNet.Serialization;

namespace CNC.NCO.Launcher.Model;

public class DiscImageSource
{
  public int SortOrder { get; set; }
  public string Name { get; set; }
  public string? DisplayName { get; set; }
  [YamlIgnore]
  public string DisplayNameOrName => DisplayName ?? Name;
  public bool Enabled { get; set; }
  public DiscImageSourceConfig Config { get; set; }
  public GameDataConfig Game { get; set; }
  public string? SplashScreenFile { get; set; }
  public Dictionary<string, List<string>> Provides  { get; set; }

  public override bool Equals(object? obj)
  {
    return DisplayNameOrName.Equals((obj as DiscImageSource)?.DisplayNameOrName);
  }
}
