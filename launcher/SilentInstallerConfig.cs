using PowerArgs;

namespace CNC.NCO.Launcher;

internal class SilentInstallerConfig
{
  [ArgShortcut("--silent-install")]
  [ArgShortcut(ArgShortcutPolicy.ShortcutsOnly)]
  [ArgDescription("Install NCO without showing a desktop window")]
  [DefaultValue(false)]
  public bool SilentInstall { get; set; }

  [ArgShortcut("--install-path")]
  [ArgShortcut(ArgShortcutPolicy.ShortcutsOnly)]
  [ArgDescription("Destination directory, defaults to local app data")]
  public string? InstallPath { get; set; }

  [ArgShortcut("--install-all")]
  [ArgShortcut(ArgShortcutPolicy.ShortcutsOnly)]
  [ArgDescription("Install Tiberian Dawn, Red Alert and The Lost Files patch for Red Alert")]
  [DefaultValue(false)]
  public bool InstallAll { get; set; }

  [ArgShortcut("--install-td")]
  [ArgShortcut(ArgShortcutPolicy.ShortcutsOnly)]
  [ArgDescription("Install Tiberian Dawn")]
  [DefaultValue(false)]
  public bool InstallTd { get; set; }

  [ArgShortcut("--install-ra")]
  [ArgShortcut(ArgShortcutPolicy.ShortcutsOnly)]
  [ArgDescription("Install Red Alert")]
  [DefaultValue(false)]
  public bool InstallRa { get; set; }

  [ArgShortcut("--install-ra-tlf")]
  [ArgShortcut(ArgShortcutPolicy.ShortcutsOnly)]
  [ArgDescription("Install The Lost Files patch for Red Alert")]
  [DefaultValue(false)]
  public bool InstallRaTheLostFiles { get; set; }
}
