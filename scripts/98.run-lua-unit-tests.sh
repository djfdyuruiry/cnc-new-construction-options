#! /usr/bin/env bash
set -eEuo pipefail

script_path="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

"${script_path}/bin/lua" -e 'require("tests.unit")'
