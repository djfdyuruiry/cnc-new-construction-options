namespace CNC.NCO.Launcher;

public record DiscImageSource(
    string Faction,
    string Url,
    bool IsZipped,
    string ImageFileName,
    string[] Files,
    string? StripFilePrefix = null
);
