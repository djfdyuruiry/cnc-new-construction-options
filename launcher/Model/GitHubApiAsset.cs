using System.Text.Json.Serialization;

namespace CNC.NCO.Launcher.Model;

public class GitHubApiAsset
{
    [JsonPropertyName("name")]
    public string? Name { get; set; }

    [JsonPropertyName("browser_download_url")]
    public string? BrowserDownloadUrl { get; set; }
}
