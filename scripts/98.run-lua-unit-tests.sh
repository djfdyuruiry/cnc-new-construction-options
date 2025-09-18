#! /usr/bin/env bash
# shellcheck source-path=SCRIPTDIR
set -eEuo pipefail

script_path="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

. "${script_path}/lib/vars.sh"
. "${script_path}/lib/functions.sh"

function main() {
  EXTRA_LUA_PATH="${test_lua_scripts_path}/?.lua;${test_lua_scripts_path}/?/init.lua" \
    "${scripts_bin_path}/lua" \
    "${test_lua_scripts_path}/tests/unit/init.lua"
}

main
