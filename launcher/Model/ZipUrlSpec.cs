using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;

using YamlDotNet.Serialization;

namespace CNC.NCO.Launcher.Model;

public class ZipUrlSpec : INotifyPropertyChanged
{
  private bool _enabled;

  public int SortOrder { get; set; }
  public string Name { get; set; }
  public string? DisplayName { get; set; }
  [YamlIgnore]
  public string DisplayNameOrName => DisplayName ?? Name;
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
  [YamlIgnore]
  public bool HasInfoUrl => InfoUrl is not null;
  [YamlIgnore]
  public GameDataConfig Game { get; set; }

  public event PropertyChangedEventHandler? PropertyChanged;

  public ZipUrlSpec()
  {
    Enabled = true;
  }

  protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null)
  {
    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
  }

  public override bool Equals(object? obj)
  {
    return DisplayNameOrName.Equals((obj as  ZipUrlSpec)?.DisplayNameOrName, StringComparison.Ordinal);
  }
}
