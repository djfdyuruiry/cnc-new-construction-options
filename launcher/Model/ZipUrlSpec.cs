namespace CNC.NCO.Launcher.Model;

public class ZipUrlSpec
{
  public string? DisplayName { get; set; }
  public required string Url { get; set; }
  public required string ProvidesFilesEndingWith { get; set; }
}
