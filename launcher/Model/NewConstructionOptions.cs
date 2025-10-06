using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using YamlDotNet.Serialization;

namespace CNC.NCO.Launcher.Model;

public class NewConstructionOptions : INotifyPropertyChanged
{
  private string? _installPath;

  public GitHubRepo GitHubRepo { get; set; }
  public string Release { get; set; }
  public string AssetPrefix { get; set; }
  public string? InstallPath
  {
    get => _installPath;
    set
    {
      _installPath = value;
      OnPropertyChanged();
    }
  }
  public bool? Installed { get; set; }
  [YamlIgnore]
  public bool IsInstalled => Installed ?? false;
  [YamlIgnore]
  public bool HasInstallPath => !string.IsNullOrEmpty(InstallPath);

  public event PropertyChangedEventHandler? PropertyChanged;

  protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null)
  {
    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
  }
}
