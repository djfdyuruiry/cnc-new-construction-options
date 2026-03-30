#! /usr/bin/env bash
# shellcheck source-path=SCRIPTDIR
set -eEuo pipefail

script_path="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

. "${script_path}/lib/vars.sh"

# Read log file usage example: cat "$(./scripts/89.get-remaster-log-path.sh)"
function main() {
  set -Eeuo pipefail

  printf '%s' "$(protontricks --no-runtime -c 'echo "${WINEPREFIX}"' "${remaster_steam_id}")/drive_c/users/steamuser/Documents/CnCRemastered/Mods/Tiberian_Dawn/NCO_TD/Data/nco.log"
}

main "$@"
