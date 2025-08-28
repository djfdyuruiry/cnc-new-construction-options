#!/usr/bin/env bash
# shellcheck source-path=SCRIPTDIR
set -eEuo pipefail

script_path="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

. "${script_path}/lib/vars.sh"
. "${script_path}/lib/functions.sh"

function main() {
  local cmake_preset="${1}"
  local source="${2}"
  local target="${3}"

  log_info "Exanding deploy target path using .env file (if present): ${env_file_path}"
  load_env_file_if_present

  target="$(eval "echo ${target}")"

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

  rm -fv "$(dirname "${target}")/"*.log

}

main "$@"
