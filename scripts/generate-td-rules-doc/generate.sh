#! /usr/bin/env bash
# shellcheck source-path=SCRIPTDIR
set -eEuo pipefail

script_path="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

function main() {
  cd "$(mktemp -d)"

  python -m venv .
  . "bin/activate"

  python -m ensurepip
  pip install -r "${script_path}/requirements.txt"

  python "${script_path}/generate.py"

  rm -rf "$(pwd)"
}

main "$@"
