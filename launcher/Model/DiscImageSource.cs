using System.Collections.Generic;

namespace CNC.NCO.Launcher.Model;

public class DiscImageSource
{
  public string Name { get; set; }
  public DiscImageSourceConfigWithName Config { get; set; }
  public Dictionary<string, List<string>> Provides  { get; set; }
}
