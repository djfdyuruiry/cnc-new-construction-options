#! /usr/bin/env bash
set -eEuo pipefail

script_path="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

. "${script_path}/../lib/functions.sh"
. "${script_path}/../lib/vars.sh"

function run_generate_python() {
  cd "$(mktemp -d)"

  python -m venv .
  . "bin/activate"

  python -m ensurepip
  pip install -r "${script_path}/requirements.txt"

  python "${script_path}/generate.py"

  rm -rf "$(pwd)"
}

function main() {
  local lua_language_server_path="${1:-}"

  if [ -z "${lua_language_server_path}" ]; then
    # shellcheck disable=SC2125
    lua_language_server_path="$(printf '%s' "${HOME}/.vscode/extensions/"sumneko.lua*"/server")"
  fi

  local lua_language_server_bin="${lua_language_server_path}/bin/lua-language-server"

  if [ ! -x "${lua_language_server_bin}" ]; then
    error_and_exit "Unable to find Lua Language Server binary, it is required to execute this script"
  fi

  "${lua_language_server_bin}" "--doc=${repo_path}" "--doc_out_path=${script_path}"
  run_generate_python
}

main "$@"
