using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using YamlDotNet.Serialization;

namespace CNC.NCO.Launcher.Model;

public class DiscImage : INotifyPropertyChanged
{
  public static readonly string RootPlaceholder = "_root";

  private bool _enabled;

  public int SortOrder { get; set; }
  public string Name { get; set; }
  public string? DisplayName { get; set; }
  [YamlIgnore]
  public string DisplayNameOrName => DisplayName ?? Name;
  public bool Required { get; set; }
  [YamlIgnore]
  public bool IsOptional => !Required;
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
  public Dictionary<string, List<string>> Provides { get; set; }

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
      s => new DiscImageSource
      {
        SortOrder = SortOrder,
        Name = Name,
        DisplayName = DisplayName,
        Enabled = Enabled,
        Config = s.Value,
        Game = gameDataConfig,
        SplashScreenFile = SplashScreenFile,
        Provides = Provides
      }
    );
  }

  public event PropertyChangedEventHandler? PropertyChanged;

  protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null)
  {
    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
  }
}
