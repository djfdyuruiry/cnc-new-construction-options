#! /usr/bin/env bash
set -eEuo pipefail

script_path="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

ROOT_PATH="$(realpath "${script_path}/../..")"
ANALYZER_PATH="${ROOT_PATH}/.clang-build-analyzer"

function main()
{
  local preset="${1}"
  local cmakeFile="${2}"
  local target="${3}"

  local buildPath="${ROOT_PATH}/build/${preset}/${cmakeFile}/CMakeFiles/${target}.dir"

  if [ ! -d "${buildPath}" ]; then
    echo "ERROR: Build path was not found, have you ran the build?"
    exit 1
  fi

  if [ ! -d "${buildPath}" ]; then
    echo "ERROR: Build path was not found, have you ran the build?"
    exit 1
  fi

  mkdir -p "${ANALYZER_PATH}"

  local binFile="${ANALYZER_PATH}/${target}.bin"

  pushd "${ANALYZER_PATH}" &> /dev/null

  rm -f "${binFile}"
  ClangBuildAnalyzer --all "${buildPath}" "${binFile}"

  local outputFile="${ANALYZER_PATH}/clang-build-analyzer_${target}.txt"

  rm -f "${outputFile}"
  ClangBuildAnalyzer --analyze "${binFile}" > "${outputFile}"

  popd &> /dev/null

  echo "Build Analysis File: ${outputFile}"
}

main "$@"
