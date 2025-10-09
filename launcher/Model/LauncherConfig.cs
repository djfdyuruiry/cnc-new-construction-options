using System.Collections.Generic;
using System.Linq;
using YamlDotNet.Serialization;

namespace CNC.NCO.Launcher.Model;

#pragma warning disable CS8618 // YAML deserialization populates these members
public class LauncherConfig
{
  [YamlMember(Alias = "$schema")]
  public string Schema { get; set; }
  public NewConstructionOptions NCO { get; set; }
  public GameDataConfig TiberianDawn { get; set; }
  public GameDataConfig RedAlert { get; set; }
  
  // virtual properties for processing and view models
  [YamlIgnore]
  public IEnumerable<GameDataConfig> Games => 
    new List<GameDataConfig> { TiberianDawn, RedAlert }.OrderBy(g => g.SortOrder);

  [YamlIgnore]
  public IEnumerable<DiscImage> DiscImages =>
    Games.SelectMany(g => 
      g.DiscImages.OrderBy(d => d.SortOrder)
    );

  [YamlIgnore]
  public IEnumerable<DiscImageSource> DiscImageSources =>
    Games.SelectMany(g => 
      g.DiscImagesBySource
        .First()
        .Value
        .OrderBy(d => d.SortOrder)
    );

  [YamlIgnore]
  public IEnumerable<ZipUrlSpec> ZipUrlSpecs => 
    Games.SelectMany(g => 
      (g.ZipUrls ?? []).OrderBy(z => z.SortOrder)
    );

  [YamlIgnore]
  public IEnumerable<GameDataConfig> EnabledGames => Games.Where(g => g.Enabled);

  [YamlIgnore]
  public IEnumerable<DiscImage> EnabledDiscImages =>
    EnabledGames
      .SelectMany(g => 
        g.DiscImages
          .Where(d => d.Enabled)
          .OrderBy(d => d.SortOrder)
      );

  [YamlIgnore]
  public IEnumerable<DiscImageSource> EnabledDiscImageSources =>
    EnabledGames
      .SelectMany(g => 
        g.DiscImagesBySource
          .First()
          .Value
          .Where(d => d.Enabled)
          .OrderBy(d => d.SortOrder)
      );
  
  [YamlIgnore]
  public IEnumerable<ZipUrlSpec> EnabledZipUrlSpecs =>
    EnabledGames
      .SelectMany(g => 
        (g.ZipUrls?.Where(z => z.Enabled) ?? []).OrderBy(z => z.SortOrder)
      );
}
