using System.Collections.Generic;
using Avalonia.Remote.Protocol.Viewport;

namespace CNC.NCO.Launcher.Model;

public class GameDataConfig
{
  public string NcoZipPath { get; set; }
  public string InstallPostfix { get; set; }
  public Dictionary<string, DiscImage> DiscImages { get; set; }
}
