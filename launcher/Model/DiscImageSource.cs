using System;
using System.Collections.Generic;

using YamlDotNet.Serialization;

namespace CNC.NCO.Launcher.Model;

#pragma warning disable CS8618 // YAML deserialization populates these members
public class DiscImageSource(DiscImage image)
{
  public int SortOrder => image.SortOrder;
  public string Name => image.Name;
  public string? DisplayName => image.DisplayName;
  public bool Enabled => image.Enabled;
  public DiscImageSourceConfig Config { get; set; }
  public GameDataConfig Game { get; set; }
  public string? SplashScreenFile => image.SplashScreenFile;
  public string? SetupPackageFile => image.SetupPackageFile;
  public Dictionary<string, List<string>> Provides => image.Provides;

  // virtual properties for processing and view models
  [YamlIgnore]
  public string DisplayNameOrName => DisplayName ?? Name;
  [YamlIgnore]
  public bool HasSplashScreenFile => !string.IsNullOrWhiteSpace(SplashScreenFile);
  [YamlIgnore]
  public bool HasSetupPackageFile => !string.IsNullOrWhiteSpace(SetupPackageFile);

  public override bool Equals(object? obj) => 
    DisplayNameOrName.Equals((obj as DiscImageSource)?.DisplayNameOrName, StringComparison.Ordinal);

  public override int GetHashCode() => HashCode.Combine(DisplayName);
}
