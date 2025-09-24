$ErrorActionPreference = "Stop"

$scriptPath = $PSScriptRoot
$repoPath = Join-Path $scriptPath "../.."

function Main {
  Push-Location -Path $repoPath

  $vcpkgPath = Join-Path $repoPath ".vcpkg"
  $env:VCPKG_ROOT = $vcpkgPath

  if (-Not (Test-Path -Path "${vcpkgPath}/scripts/bootstrap.ps1" -PathType Leaf)) {
    git submodule update --init --recursive

    if ($LASTEXITCODE -ne 0) {
      throw "vcpkg repo pull failed with exit code: $LASTEXITCODE"
    }
  }

  if (-Not (Test-Path -Path "${vcpkgPath}/vcpkg" -PathType Leaf)) {
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File "${vcpkgPath}/scripts/bootstrap.ps1" -disableMetrics

    if ($LASTEXITCODE -ne 0) {
      throw "vcpkg bootstrap failed with exit code: $LASTEXITCODE"
    }
  }

  if ($args[0] -eq "--init") {
    return
  }

  & "${vcpkgPath}/vcpkg" @args

  if ($LASTEXITCODE -ne 0) {
    throw "vcpkg failed with exit code: $LASTEXITCODE"
  }

  Pop-Location
}

Main @args
