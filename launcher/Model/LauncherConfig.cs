using System.Collections.Generic;
using System.Linq;
using YamlDotNet.Serialization;

namespace CNC.NCO.Launcher.Model;

#pragma warning disable CS8618 // YAML deserialization populates these members
public class LauncherConfig
{
  public const string LauncherDirectory = "launcher";

  [YamlMember(Alias = "$schema")]
  public string Schema { get; set; }
  [YamlMember(Alias = "NCO")]
  public NewConstructionOptions Nco { get; set; }
  public GameDataConfig TiberianDawn { get; set; }
  public GameDataConfig RedAlert { get; set; }

  // virtual properties for processing and view models
  [YamlIgnore]
  public IEnumerable<GameDataConfig> Games =>
    new List<GameDataConfig> { TiberianDawn, RedAlert }.OrderBy(g => g.SortOrder);

  [YamlIgnore]
  public IEnumerable<GameDataConfig> EnabledGames => Games.Where(g => g.Enabled);

  [YamlIgnore]
  public IEnumerable<DiscImageSource> EnabledDiscImages => EnabledGames.SelectMany(g => g.EnabledPrimaryDiscImages);

  [YamlIgnore]
  public IEnumerable<ZipUrlSpec> EnabledZipUrlSpecs => EnabledGames.SelectMany(g => g.EnabledZipUrlSpecs);
}
