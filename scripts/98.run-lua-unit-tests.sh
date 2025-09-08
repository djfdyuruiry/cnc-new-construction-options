#! /usr/bin/env bash
# shellcheck source-path=SCRIPTDIR
set -eEuo pipefail

script_path="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

. "${script_path}/lib/vars.sh"

export EXTRA_LUA_PATH="${test_lua_scripts_path}/?.lua;${test_lua_scripts_path}/?/init.lua"

"${script_path}/bin/lua" -e 'require("tests.unit")'
