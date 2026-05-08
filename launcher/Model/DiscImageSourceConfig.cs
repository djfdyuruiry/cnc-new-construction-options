using YamlDotNet.Serialization;

namespace CNC.NCO.Launcher.Model;

#pragma warning disable CS8618 // YAML deserialization populates these members
public class DiscImageSourceConfig
{
  public int SortOrder { get; set; }
  public string Name { get; set; }
  public string DisplayName { get; set; }
  public string Url { get; set; }
  public string File { get; set; }
  public string Checksum { get; set; }

  // virtual properties for processing and view models
  [YamlIgnore]
  public bool IsZipSource => Url.EndsWith(".zip");
}
