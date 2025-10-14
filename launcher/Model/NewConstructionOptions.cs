using System.ComponentModel;
using System.Runtime.CompilerServices;

using YamlDotNet.Serialization;

namespace CNC.NCO.Launcher.Model;

#pragma warning disable CS8618 // YAML deserialization populates these members
public class NewConstructionOptions : INotifyPropertyChanged
{
  private string _installPath = string.Empty;
  private bool _installed = false;

  private string _pendingInstallPath;

  public GitHubRepo GitHubRepo { get; set; }
  public string Release { get; set; }
  public string AssetPrefix { get; set; }
  public string InstallPath
  {
    get => _installPath;
    set
    {
      _installPath = value;
      OnPropertyChanged();
    }
  }

  public bool Installed
  {
    get => _installed;
    set
    {
      _installed = value;
      OnPropertyChanged();
    }
  }

  public string MsvcRuntimeUrl { get; set; }

  /**
   * Virtual property by installer flow to hold path in a temp variable, commited
   * to InstallPath when install succeeds.
   */
  [YamlIgnore]
  public string PendingInstallPath
  {
    get => _pendingInstallPath;
    set
    {
      _pendingInstallPath = value;
      OnPropertyChanged();
    }
  }

  public event PropertyChangedEventHandler? PropertyChanged;

  protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null) =>
    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
}
