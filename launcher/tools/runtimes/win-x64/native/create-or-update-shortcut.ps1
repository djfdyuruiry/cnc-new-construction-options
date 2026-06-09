param(
  [Parameter(Mandatory=$true)]
  [string]
  $ShortcutPath,
  [Parameter(Mandatory=$true)]
  [string]
  $TargetPath,
  [Parameter(Mandatory=$false)]
  [string]
  $ShortcutIconPath = ""
)

$ErrorActionPreference="Stop"

if (-not $ShortcutPath.EndsWith(".lnk")) {
  $ShortcutPath = "${ShortcutPath}.lnk"
}

$shell = New-Object -COM WScript.Shell

$shortcut = $shell.CreateShortcut($ShortcutPath)

$shortcut.TargetPath = $TargetPath
$shortcut.WorkingDirectory = Split-Path -Path $TargetPath

if (-not [string]::IsNullOrWhitespace($ShortcutIconPath)) {
  $shortcut.IconLocation = $ShortcutIconPath
}

$shortcut.Save()
