#! /usr/bin/env bash
set -Eexuo pipefail

GITHUB_RELEASE_URL="https://api.github.com/repos/djfdyuruiry/cnc-new-construction-options/releases"

function error_and_exit() {
  set -eEuo pipefail

  local error="${1}"

  >&2 echo "!ERROR! ${error}"
  exit 1
}

function install_mod() {
  set -eEuo pipefail

  local mod_name="${1}"
  local mod_zip_path="${2}"
  local install_path="${3}"

  echo ">> Installing ${mod_name} to ${install_path}..."

  mkdir -p "${install_path}"
  unzip -qo "${mod_zip_path}" -d "${install_path}"
}

function download_mod() {
  set -eEuo pipefail

  local asset_starts_with="${1}"
  local mod_zip_path="${2}"

  echo ">> Downloading '${asset_starts_with}' mod zip..."

  if ! command -v "curl" &> /dev/null; then
    error_and_exit "Please install curl using your OS package manager, it is required to download NCO mods"
  fi

  local github_releases
  github_releases="$(curl -s --show-error --fail-with-body --location "${GITHUB_RELEASE_URL}")"

  local mod_zip_url
  mod_zip_url="$(echo "${github_releases}" | grep "browser_download_url" | grep "${asset_starts_with}" | grep ".zip" | cut -d '"' -f 4)"

  curl --location -s --show-error --fail-with-body -o "${mod_zip_path}" "${mod_zip_url}"
}

function download_and_install_mods() {
  set -eEuo pipefail

  local pfx_path="${1}"

  local mods_path="${pfx_path}/drive_c/users/steamuser/Documents/CnCRemastered/Mods"
  echo "Mods path: ${mods_path}"

  # Tiberian Dawn
  local td_mod_zip_path
  td_mod_zip_path="$(mktemp)"
  local td_path="${mods_path}/Tiberian_Dawn"

  download_mod "nco-td-remaster-mod-msvc" "${td_mod_zip_path}"
  install_mod "NCO_TD" "${td_mod_zip_path}" "${td_path}"

  # Red Alert
  local ra_mod_zip_path
  ra_mod_zip_path="$(mktemp)"
  local ra_path="${mods_path}/Red_Alert"

  download_mod "nco-ra-remaster-mod-msvc" "${ra_mod_zip_path}"
  install_mod "NCO_RA" "${ra_mod_zip_path}" "${ra_path}"
}

# we might need to give protontricks access to the steam library where C&C lives
function flatpak_permissions_check() {
  set -eEuo pipefail

  local protontricks_cmd="${1}"

  echo ">> Checking Flatpak Permissions"

  local flatpak_log
  flatpak_log="$(mktemp)"

  ${protontricks_cmd} -l &> "${flatpak_log}"

  if grep "Protontricks does not appear to have access to the following directories" "${flatpak_log}" &> /dev/null; then
    eval "$(grep "flatpak" "${flatpak_log}")"
  fi
}

function resolve_protontricks() {
  set -eEuo pipefail

  local flatpak_app="com.github.Matoking.protontricks"

  if command -v protontricks &> /dev/null; then
    echo "protontricks"
  elif flatpak info "${flatpak_app}" &> /dev/null; then
    flatpak_permissions_check "flatpak run ${flatpak_app}" 1>&2

    echo "flatpak run ${flatpak_app}"
  else
    error_and_exit "Unable to find protontricks. Please install it, see: https://github.com/Matoking/protontricks?tab=readme-ov-file#installation"
  fi
}

function main()
{
  set -eEuo pipefail

  local game_name="Command & Conquer™ Remastered Collection"
  local game_id="1213210"

  echo '
   █████████    ██████     █████████        ██████   █████   █████████     ███████
  ███░░░░░███  ███░░███   ███░░░░░███      ░░██████ ░░███   ███░░░░░███  ███░░░░░███
 ███     ░░░  ░░██████   ███     ░░░  ██    ░███░███ ░███  ███     ░░░  ███     ░░███
░███           ██████   ░███         ░░     ░███░░███░███ ░███         ░███      ░███
░███         ░███░░███  ░███                ░███ ░░██████ ░███         ░███      ░███
░░███     ███░███ ░░███ ░░███     ███       ░███  ░░█████ ░░███     ███░░███     ███
 ░░█████████ ░░█████░███ ░░█████████  ██    █████  ░░█████ ░░█████████  ░░░███████░
  ░░░░░░░░░   ░░░░░ ░░░   ░░░░░░░░░  ░░    ░░░░░    ░░░░░   ░░░░░░░░░     ░░░░░░░

                                                                                     '

  echo ">> Resolving protontricks"

  local protontricks_cmd
  protontricks_cmd="$(resolve_protontricks)"

  echo ">> Checking '${game_name}' is installed"

  if ! ${protontricks_cmd} -s "${game_name}" | grep "(${game_id})" &> /dev/null; then
    error_and_exit "'${game_name}' is not installed, please install it via Steam and start it at least once"
  fi

  echo ">> Resolving '${game_name}' install path"

  local pfx_path
  pfx_path="$(${protontricks_cmd} --no-runtime -c 'echo "${WINEPREFIX}"' "${game_id}")"

  # Assert C&C has already created folders (this is done on first launch)
  if [ ! -d "${pfx_path}" ]; then
    echo "!ERROR! Please open Steam and run C&C: Remastered at least once before attempting to install this mod"
    exit 1
  fi

  echo ">> Installing Microsoft Visual C++ Redistributable in Proton"
  ${protontricks_cmd} --no-runtime "${game_id}" --unattended vcrun2022

  download_and_install_mods "${pfx_path}"

  echo ">> Installation Complete!"

  xdg-open "${mods_path}"
}

main "$@"
