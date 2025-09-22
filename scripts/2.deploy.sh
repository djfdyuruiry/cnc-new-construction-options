#!/usr/bin/env bash
# shellcheck source-path=SCRIPTDIR
set -eEuo pipefail

script_path="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

. "${script_path}/lib/vars.sh"
. "${script_path}/lib/functions.sh"

function deploy_test_files() {
  if [ "${cmake_preset}" == "nco-tiberian-dawn-debug" ]; then
    log_info "Deploying TD test files"
    cp -rfv "${td_test_resources_path}"/* "${target_dir}"
  fi
}

function main() {
  local cmake_preset="${1}"
  local build_sub_dir="${2}"
  local binary="${3}"
  local target_binary="${4}"

  log_info "Expanding deploy target path using .env file (if present): ${env_file_path}"
  load_env_file_if_present

  target_binary="$(eval "echo ${target_binary}")"
  target_dir="$(dirname "${target_binary}")"

  local build_type="Debug"

  if ! [[ "${cmake_preset}" =~ ^nco.+-debug$ ]]; then
    build_type="RelWithDebInfo"
  fi

  local build_output="${build_path}/${cmake_preset}/${build_type}/${build_sub_dir}"

  log_info "Deploying Game Binary"
  cp -fv "${build_output}/${binary}" "${target_binary}"

  log_info "Deploying Game Data Files"
  mv -f "${build_output}/$(basename "${binary}")" "/tmp"
  cp -rfv "${build_output}"/* "${target_dir}"
  mv -f "/tmp/$(basename "${binary}")" "${build_output}"

  deploy_test_files

  rm -fv "${target_dir}/"*.log
}

main "$@"
