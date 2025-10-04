namespace CNC.NCO.Launcher.Model;

public class DiscImageSourceConfig
{
  public string Name { get; set; }
  public string DisplayName { get; set; }
  public string Url { get; set; }
  public string File { get; set; }
  public bool IsZipSource => Url.EndsWith(".zip");
}
