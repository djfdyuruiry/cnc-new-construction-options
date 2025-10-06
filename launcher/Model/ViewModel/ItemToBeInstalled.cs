using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace CNC.NCO.Launcher.Model.ViewModel;

public class ItemToBeInstalled<T>(T item) : INotifyPropertyChanged
{
  private bool _errored;
  private bool _installing;
  private bool _installed;

  public T Item { get; } = item;

  public bool Installing
  {
    get => _installing;
    set
    {
      _installing = value;
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

  public bool Errored
  {
    get => _errored;
    set
    {
      _errored = value;
      OnPropertyChanged();
    }
  }

  public event PropertyChangedEventHandler? PropertyChanged;

  protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null) =>
    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
}
