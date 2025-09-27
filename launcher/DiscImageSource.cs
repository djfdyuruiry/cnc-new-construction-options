namespace cnc_installer_poc;

public record DiscImageSource(string Faction, string Url, bool IsZipped, string ImageFileName, string[] Files, string? StripFilePrefix = null);
