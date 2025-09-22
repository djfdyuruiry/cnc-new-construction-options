#!/usr/bin/env bash
# shellcheck source-path=SCRIPTDIR
set -eEuo pipefail

script_path="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

function main() {
  # create build image
  docker build --file "${script_path}/ubuntu.Dockerfile" \
    --build-arg "UID=$(id -u)" \
    --build-arg "GID=$(id -g)" \
    -t nco-build .

  # init src (if missing)
  mkdir -p "${script_path}/src"

  if ! [ -d "${script_path}/src/.git" ]; then
    # fetch a clean repo
    git clone https://github.com/djfdyuruiry/cnc-new-construction-options.git "${script_path}/src" \
      && pushd "${script_path}/src" \
      && git checkout feat/cmake_pull-sdl2-openal-from-vcpkg \
      && git pull \
      && popd
  fi

  # run the build
  rm -rf "${script_path:?}/bin"
  docker run --name nco-run-build -it -v "${script_path}/src:/build" nco-build:latest || {
    echo "ERROR: Docker build failed"
    echo "Source path: ${script_path}/src"
    exit 1
  }

  # extract build output
  mkdir -p "${script_path:?}/bin"
  docker cp nco-run-build:/build/build/nco/RelWithDebInfo/. "${script_path}/bin/"

  # cleanup container
  docker rm nco-run-build

  echo "Source path: ${script_path}/src"
  echo "Binary output path: ${script_path}/bin"
}

main "$@"
