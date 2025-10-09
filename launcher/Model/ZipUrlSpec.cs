using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;

using YamlDotNet.Serialization;

namespace CNC.NCO.Launcher.Model;

#pragma warning disable CS8618 // YAML deserialization populates these members
public class ZipUrlSpec : INotifyPropertyChanged
{
  private bool _enabled;

  public int SortOrder { get; set; }
  public string Name { get; set; }
  public string? DisplayName { get; set; }
  public bool Enabled
  {
    get => _enabled;
    set
    {
      _enabled = value;
      OnPropertyChanged();
    }
  }
  public required string Url { get; set; }
  public required string ProvidesFilesEndingWith { get; set; }
  public string? InfoUrl { get; set; }

  // virtual properties for processing and view models
  [YamlIgnore]
  public string DisplayNameOrName => DisplayName ?? Name;
  [YamlIgnore]
  public bool HasInfoUrl => InfoUrl is not null;
  [YamlIgnore]
  public GameDataConfig Game { get; set; }

  public event PropertyChangedEventHandler? PropertyChanged;

  public ZipUrlSpec()
  {
    Enabled = true;
  }

  protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null) => 
    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));

  public override bool Equals(object? obj) => 
    DisplayNameOrName.Equals((obj as  ZipUrlSpec)?.DisplayNameOrName, StringComparison.Ordinal);

  public override int GetHashCode() => HashCode.Combine(DisplayNameOrName);
}
