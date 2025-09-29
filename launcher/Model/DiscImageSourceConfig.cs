namespace CNC.NCO.Launcher.Model;

public class DiscImageSourceConfig
{
  public string Url { get; set; }
  public string File { get; set; }
  public bool IsZipSource => Url.EndsWith(".zip");

  public DiscImageSourceConfigWithName WithName(string name) =>
    new()
    {
      Name = name,
      File = File,
      Url = Url
    };
}
