using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;

namespace CNC.NCO.Launcher.Model;

public class GameDataConfig : INotifyPropertyChanged
{
  private bool _enabled;
  public string DisplayName { get; set; }
  public string NcoZipPath { get; set; }
  public string InstallPostfix { get; set; }
  public DiscImage[] DiscImages { get; set; }
  public ZipUrlSpec[]? ZipUrls { get; set; }
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

  public GameDataConfig()
  {
    // TODO: persist back to yml
    Enabled = true;
  }

  public Dictionary<string, List<DiscImageSource>> DiscImagesBySource => DiscImages
    .SelectMany(x => x.Sources.Select(s => s.Name))
    .Distinct()
    .ToDictionary(
      p => p,
      p => DiscImages
        .Select(d => d.BuildSources().GetValueOrDefault(p))
        .Where(d => d is not null)
        .Select(d => d!)
        .ToList()
    );

  public event PropertyChangedEventHandler? PropertyChanged;

  protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null)
  {
    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
  }
}
