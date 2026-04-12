$ErrorActionPreference = "Stop"

$GithubReleaseUrl = "https://api.github.com/repos/djfdyuruiry/cnc-new-construction-options/releases"
$MsvcInstallerUrl = "https://aka.ms/vc14/vc_redist.x86.exe"

function Install-Mod() {
  param(
    [string] $modName,
    [string] $modZipPath,
    [string] $installPath
  )

  $install_name = Split-Path $installPath -Leaf

  Write-Host ">> Installing ${modName} to ${install_name}..."

  if (-not (Test-Path $installPath)) {
    Write-Host "Creating ${install_name} mods folder..."
    New-Item -ItemType Directory -Path $installPath -Force
  }

  Expand-Archive -Path $modZipPath -DestinationPath $installPath -Force
  Remove-Item -Force $modZipPath -ErrorAction Ignore

  Write-Host ">> Unblocking all mod files..."

  $files = Get-ChildItem -Path $installPath -Recurse -File -Force

  foreach ($file in $files) {
    Unblock-File -Path $file.FullName
  }
}

function Download-Mod-Zip() {
  param(
    [string] $assetStartsWith,
    [string] $modZipPath
  )
  Write-Host ">> Downloading '${assetStartsWith}' mod zip..."

  $releaseAssets = $(Invoke-RestMethod -Uri $GithubReleaseUrl).assets

  $modZipUrl = $releaseAssets | Where-Object { $_.name.StartsWith($assetStartsWith) } | Select-Object -ExpandProperty url

  Invoke-WebRequest -Uri $modZipUrl -OutFile $modZipPath -UseBasicParsing
}

function Install-Msvc() {
  $msvcUrl = $MsvcInstallerUrl
  $msvcInstaller = New-TemporaryFile

  Write-Host ">> Downloading Microsoft Visual C++ Redistributable..."
  Invoke-WebRequest -Uri $msvcUrl -OutFile $msvcInstaller -UseBasicParsing

  Write-Host ">> Unblocking redistributable exe..."
  Unblock-File $msvcInstaller

  Write-Host ">> Installing redistributable..."
  Start-Process -FilePath $msvcInstaller -ArgumentList "/quiet /norestart" -Wait

  Remove-Item -Force $msvcInstaller -ErrorAction Ignore
}

function Main() {
  Write-Host '
   █████████    ██████     █████████        ██████   █████   █████████     ███████
  ███░░░░░███  ███░░███   ███░░░░░███      ░░██████ ░░███   ███░░░░░███  ███░░░░░███
 ███     ░░░  ░░██████   ███     ░░░  ██    ░███░███ ░███  ███     ░░░  ███     ░░███
░███           ██████   ░███         ░░     ░███░░███░███ ░███         ░███      ░███
░███         ░███░░███  ░███                ░███ ░░██████ ░███         ░███      ░███
░░███     ███░███ ░░███ ░░███     ███       ░███  ░░█████ ░░███     ███░░███     ███
 ░░█████████ ░░█████░███ ░░█████████  ██    █████  ░░█████ ░░█████████  ░░░███████░
  ░░░░░░░░░   ░░░░░ ░░░   ░░░░░░░░░  ░░    ░░░░░    ░░░░░   ░░░░░░░░░     ░░░░░░░

                                                                                     '

  $modsPath = "$($env:USERPROFILE)\Documents\CnCRemastered\Mods"

  Install-Msvc

  # Tiberian Dawn
  $ncoTdModZipPath = New-TemporaryFile
  $tdPath = "$modsPath\Tiberian_Dawn"

  Download-Mod-Zip "nco-remaster-td-mod-msvc" $ncoTdModZipPath
  Install-Mod "NCO_TD" $ncoTdModZipPath $tdPath

  # Red Alert
  $ncoRaModZipPath = New-TemporaryFile
  $raPath = "$modsPath\Red_Alert"

  Download-Mod-Zip "nco-remaster-ra-mod-msvc" $ncoRaModZipPath
  Install-Mod "NCO_RA" $ncoRaModZipPath $raPath

  Write-Host ">> Installation Complete!"
}

Main
