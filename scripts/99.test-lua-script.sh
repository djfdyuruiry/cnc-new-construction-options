#! /usr/bin/env bash
set -eEuo pipefail

script_path="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

. "${script_path}/lib/vars.sh"
. "${script_path}/lib/functions.sh"

function main() {
  local lua_script_file="${1:-}"

  if [ -z "${lua_script_file}" ]; then
    error_and_exit "Usage: ${0} <lua_script_file> - where <lua_script_file> is a file in tiberiandawn/lua/scripts/nco/TiberianDawn"
  fi

  "${scripts_bin_path}/lua" \
    "${test_lua_scripts_path}/test-game-script.lua" \
    "${lua_script_file}"
}

main "$@"
