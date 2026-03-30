#! /usr/bin/env bash
# shellcheck source-path=SCRIPTDIR
set -eEuo pipefail

lib_scripts_path="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

repo_path="$(realpath "${lib_scripts_path}/../..")"
env_file_path="${repo_path}/.env"
build_directory="build"
build_path="${repo_path}/${build_directory}"
scripts_path="${repo_path}/scripts"
scripts_bin_path="${scripts_path}/bin"

vcpkg_path="${repo_path}/.vcpkg"

common_lua_scripts_path="${repo_path}/common/lua/scripts"
td_lua_scripts_path="${repo_path}/tiberiandawn/lua/scripts"
test_lua_scripts_path="${repo_path}/tests/lua/scripts"

td_resources_path="${repo_path}/resources/data/td"
td_test_resources_path="${repo_path}/resources/test/td"

remaster_steam_id="1213210"
