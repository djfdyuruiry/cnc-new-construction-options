#! /usr/bin/env bash
set -eEuo pipefail

script_path="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

. "${script_path}/lib/vars.sh"
. "${script_path}/lib/functions.sh"

function main() {
  local preset="${1:-}"
  local build_type="${2:-}"

  if [ -z "${RA_DATA_PATH:-}" ]; then
    load_env_file_if_present
  fi

  assert_variable_exists "RA_DATA_PATH" \
    "Set the env var RA_DATA_PATH to a directory that contains Tiberian Dawn game data and try again"

  if [[ "${preset}" =~ ^nco.* ]]; then
    # custom cmake preset passed, remove it before forwarding args to game
    shift
  fi

  if [ -z "${preset}" ]; then
    preset="nco-red-alert-debug"
  fi

  if [[ "${build_type}" =~ ^(Debug|RelWithDebInfo)$ ]]; then
    # custom build preset passed, remove it before forwarding args to game
    shift
  elif [[ "${preset}" =~ ^nco.* ]]; then
    build_type="RelWithDebInfo"
  else
    # not a build type, don't pass to the build script
    build_type=""
  fi

  "${script_path}/1.build.sh" "${preset}" "${build_type}"
  "${script_path}/2.deploy.sh" "${preset}" "vanillara" "${RA_DATA_PATH}/vanillara-dev"

  pushd_silent "${RA_DATA_PATH}"

  local log_path="${build_path}/vanillara.log"
  start_logging "${log_path}"

  local exit_code=0

  NCO_LOG_LEVEL="debug" ./vanillara-dev "$@" || {
    exit_code="$?"
    stop_logging
    log_error "Game finished with non-zero exit code: ${exit_code}"
  }

  stop_logging
  log_warning "View full game log: ${log_path}"

  rm "vanillara-dev"
  popd_silent

  exit "${exit_code}"
}

main "$@"
