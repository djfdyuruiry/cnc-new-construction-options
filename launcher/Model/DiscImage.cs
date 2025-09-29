using System.Collections.Generic;

namespace CNC.NCO.Launcher.Model;

public class DiscImage
{
  public DiscImageSourceConfig[] Sources { get; set; }

  public Dictionary<string, List<string>> Provides { get; set; }
}
