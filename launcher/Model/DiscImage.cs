using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;

using YamlDotNet.Serialization;

namespace CNC.NCO.Launcher.Model;

#pragma warning disable CS8618 // YAML deserialization populates these members
public class DiscImage : INotifyPropertyChanged
{
  public static readonly string RootPlaceholder = "_root";

  private bool _enabled;

  public int SortOrder { get; set; }
  public string Name { get; set; }
  public string? DisplayName { get; set; }
  public bool Required { get; set; }
  public bool Enabled
  {
    get => _enabled;
    set
    {
      _enabled = value;
      OnPropertyChanged();
    }
  }
  public DiscImageSourceConfig[] Sources { get; set; }
  public string? SplashScreenFile { get; set; }
  public string? SetupPackageFile { get; set; }
  public Dictionary<string, List<string>> Provides { get; set; }

  // virtual properties for processing and view models
  [YamlIgnore]
  public string DisplayNameOrName => DisplayName ?? Name;
  [YamlIgnore]
  public bool IsOptional => !Required;

  public DiscImage()
  {
    Required = false;
    Enabled = true;
  }

  public Dictionary<string, DiscImageSource> BuildSources(GameDataConfig gameDataConfig)
  {
    return Sources.ToDictionary(
      s => s.Name,
      s => s
    ).ToDictionary(
      s => s.Key,
      s => new DiscImageSource(this)
      {
        Config = s.Value,
        Game = gameDataConfig
      }
    );
  }

  public event PropertyChangedEventHandler? PropertyChanged;

  public static bool ProvidesKeyIsRoot(string key) =>
    string.Equals(RootPlaceholder, key, StringComparison.OrdinalIgnoreCase);

  protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null) => 
    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
}
