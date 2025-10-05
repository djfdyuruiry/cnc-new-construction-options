using YamlDotNet.Serialization;

namespace CNC.NCO.Launcher.Model;

public class LauncherConfig
{
  public NewConstructionOptions NCO { get; set; }
  public GameDataConfig TiberianDawn { get; set; }
  public GameDataConfig RedAlert { get; set; }
  
  [YamlIgnore]
  public GameDataConfig[] Games => [TiberianDawn, RedAlert];
}
