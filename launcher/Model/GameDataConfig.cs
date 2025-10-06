using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;

using YamlDotNet.Serialization;

namespace CNC.NCO.Launcher.Model;

public class GameDataConfig : INotifyPropertyChanged
{
  private bool _enabled;
  private ZipUrlSpec[]? _zipUrls;

  public int SortOrder { get; set; }
  public string Name { get; set; }
  public string DisplayName { get; set; }
  public string NcoZipPath { get; set; }
  public string InstallPostfix { get; set; }
  public DiscImage[] DiscImages { get; set; }

  public ZipUrlSpec[]? ZipUrls
  {
    get => _zipUrls;
    set
    {
      _zipUrls = value?.Select(z =>
      {
        z.Game = this;
        return z;
      }).ToArray();
    }
  }

  public bool Enabled
  {
    get => _enabled;
    set
    {
      if (value == _enabled) return;
      _enabled = value;
      OnPropertyChanged();
    }
  }

  [YamlIgnore]
  public Dictionary<string, List<DiscImageSource>> DiscImagesBySource => DiscImages
    .SelectMany(x => x.Sources.Select(s => s.Name))
    .Distinct()
    .ToDictionary(
      p => p,
      p => DiscImages
        .Select(d => d.BuildSources(this).GetValueOrDefault(p))
        .Where(d => d is not null)
        .Select(d => d!)
        .ToList()
    );

  public event PropertyChangedEventHandler? PropertyChanged;

  public GameDataConfig()
  {
    Enabled = true;
  }

  protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null)
  {
    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
  }
}
