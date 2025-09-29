using System.Collections.Generic;

namespace CNC.NCO.Launcher.Model;

public record DiscImageSource(
  string Name,
  string Url,
  bool IsZipped,
  string ImageFileName,
  Dictionary<string, List<string>> Files
);
