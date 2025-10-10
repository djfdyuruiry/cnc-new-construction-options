using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;

using YamlDotNet.Serialization;

namespace CNC.NCO.Launcher.Model;

#pragma warning disable CS8618 // YAML deserialization populates these members
public class GameDataConfig : INotifyPropertyChanged
{
  private bool _enabled;
  private ZipUrlSpec[]? _zipUrls;

  public int SortOrder { get; set; }
  public string Name { get; set; }
  public string DisplayName { get; set; }
  public string NcoZipPath { get; set; }
  public string InstallPostfix { get; set; }
  public string Binary { get; set; }
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
      _enabled = value;
      OnPropertyChanged();
    }
  }

  // virtual properties for processing and view models
  [YamlIgnore]
  public IList<DiscImage> OrderedDiscImages => DiscImages.OrderBy(d => d.SortOrder).ToList();

  [YamlIgnore]
  public IList<DiscImage> EnabledDiscImages => OrderedDiscImages.Where(d => d.Enabled).ToList();

  [YamlIgnore]
  public IDictionary<string, IList<DiscImageSource>> DiscImagesBySource => DiscImages
    .SelectMany(x => x.Sources.Select(s => s.Name))
    .Distinct()
    .ToDictionary(
      p => p, 
      IList<DiscImageSource> (p) => OrderedDiscImages
        .Select(d => d.BuildSources(this).GetValueOrDefault(p))
        .Where(d => d is not null)
        .Select(d => d!)
        .OrderBy(d => d.SortOrder)
        .ToList()
    );

  [YamlIgnore]
  public IDictionary<string, IList<DiscImageSource>> EnabledDiscImagesBySource => EnabledDiscImages
    .SelectMany(x => x.Sources.Select(s => s.Name))
    .Distinct()
    .ToDictionary(
      p => p,
      IList<DiscImageSource> (p) => EnabledDiscImages
        .Select(d => d.BuildSources(this).GetValueOrDefault(p))
        .Where(d => d is not null)
        .Select(d => d!)
        .OrderBy(d => d.SortOrder)
        .ToList()
    );

  [YamlIgnore]
  public IList<ZipUrlSpec> ZipUrlSpecs => (ZipUrls ?? []).OrderBy(z => z.SortOrder).ToList();

  [YamlIgnore]
  public IList<ZipUrlSpec> EnabledZipUrlSpecs => ZipUrlSpecs.Where(z => z.Enabled).ToList();

  [YamlIgnore]
  public string PlatformBinary =>
    OperatingSystem.IsWindows()
      ? $"{Binary}.exe"
      : OperatingSystem.IsMacOS()
        ? $"/Applications/{Binary}.app/Contents/MacOS/{Binary}"
        : Binary;

  public event PropertyChangedEventHandler? PropertyChanged;

  public GameDataConfig()
  {
    Enabled = true;
  }

  protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null) => 
    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
}
