#! /usr/bin/env bash
# shellcheck source-path=SCRIPTDIR
set -eEuo pipefail

script_path="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

. "${script_path}/lib/vars.sh"
. "${script_path}/lib/functions.sh"

function deploy_mod() {
  local mods_path="${1}"
  local win_mod_path="${2}"
  local game="${3}"
  local mod_postfix="${4}"

    log_info "Deploying ${mod_postfix} mod"

    local mod_path="${mods_path}/${game}/NCO_${mod_postfix}"

    rm -rf "${mod_path}"
    mkdir -p "${mod_path}"
    cp -rf "${win_mod_path}"/* "${mod_path}"
}

function main() {
  set -Eeuo pipefail

  local win_output_path="${build_path}/windows-output"
  local win_td_path="${win_output_path}/NCO_TD"
  local win_ra_path="${win_output_path}/NCO_RA"

  if [ ! -d "${win_td_path}" ] && [ ! -d "${win_ra_path}" ]; then
    error_and_exit "Ensure NCO_TD/NCO_RA directory is copied from Windows build output to ${win_output_path}"
  fi

  log_info "Ensuring MSVC redist installed in Remastered proton"
  protontricks --no-runtime "${remaster_steam_id}" --unattended vcrun2022

  local pfx_path
  pfx_path="$(protontricks --no-runtime -c 'echo "${WINEPREFIX}"' "${remaster_steam_id}")"

  local mods_path="${pfx_path}/drive_c/users/steamuser/Documents/CnCRemastered/Mods"
  echo "Mods path: ${mods_path}"

  local config_path="${pfx_path}/drive_c/users/steamuser/AppData/Roaming/nco"
  echo "Config path: ${config_path}"

  if [ -d "${win_td_path}" ]; then
    deploy_mod "${mods_path}" "${win_td_path}" "Tiberian_Dawn" "TD"

    # deploy test files
    mkdir -p "${config_path}/tiberian-dawn"
    cp -rfv "${td_test_resources_path}"/* "${config_path}/tiberian-dawn"
  fi

  if [ -d "${win_ra_path}" ]; then
    deploy_mod "${mods_path}" "${win_ra_path}" "Red_Alert" "RA"
  fi

  log_info "Launching remastered in Steam"
  steam "steam://rungameid/${remaster_steam_id}"
}

main "$@"
