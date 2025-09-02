#!/usr/bin/env bash
# shellcheck source-path=SCRIPTDIR
set -eEuo pipefail

script_path="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

. "${script_path}/lib/vars.sh"
. "${script_path}/lib/functions.sh"

function deploy_nco_lua() {
  rm -rf "${target_dir}/lua/nco"
  mkdir -p "${target_dir}/lua/nco"

  cp -rf "${common_lua_scripts_path}"/* "${target_dir}/lua/nco"

  if [ "${cmake_preset}" == "nco-tiberian-dawn-debug" ]; then
    cp -rf "${td_lua_scripts_path}"/* "${target_dir}/lua/nco"
  fi
}

function main() {
  local cmake_preset="${1}"
  local source="${2}"
  local target="${3}"

  log_info "Expanding deploy target path using .env file (if present): ${env_file_path}"
  load_env_file_if_present

  target="$(eval "echo ${target}")"
  target_dir="$(dirname "${target}")"

  local build_type="Debug"

  if [ "${cmake_preset}" == "nco" ]; then
    build_type="RelWithDebInfo"
  fi

  find "${build_path}/${cmake_preset}/${build_type}" \
    -maxdepth 1 \
    -type f \
    -executable \
    -iname "${source}" \
    -print0 | xargs -0 -I {} cp -fv "{}" "${target}"

  deploy_nco_lua

  rm -fv "${target_dir}/"*.log
}

main "$@"
