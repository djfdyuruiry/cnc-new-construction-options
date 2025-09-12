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

  local exit_code=0
  local prefix_command=""

  if [ "${PROFILE:-false}" == "true" ]; then
    if [ -z "$(command perf)" ]; then
      error_and_exit "perf command is required to run this script"
    fi

    prefix_command="perf record"
  fi

  # run with debug logging and capture profiling info
  NCO_LOG_LEVEL="debug" ${prefix_command} ./vanillara-dev "$@" || {
    exit_code="$?"
    log_error "Game finished with non-zero exit code: ${exit_code}"
  }

  log_warning "View full game log: $(pwd)/nco.log"

  rm "vanillara-dev"
  popd_silent

  exit "${exit_code}"
}

main "$@"
