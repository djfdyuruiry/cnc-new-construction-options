using System.Collections.Generic;
using System.Text.Json.Serialization;

namespace CNC.NCO.Launcher.Model;

public class GitHubApiRelease
{
    [JsonPropertyName("name")]
    public string? Name { get; set; }

    [JsonPropertyName("assets")]
    public List<GitHubApiAsset>? Assets { get; set; }
}
