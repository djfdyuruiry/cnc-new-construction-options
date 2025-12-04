##
# This Dockerfile is used to test a clean environment build, ensuring that
# all build dependencies are documented and understood. 
#
# - C++ toolchain used it mingw/gcc
# - Mirrors CI build defined in .github/workflows/mingw.yml
# - See wiki page: wiki/5.Building.md
##
FROM ubuntu:24.04

ENV LANG="C.UTF-8"

# note: jq/tree are utilities for this Dockerfile only
RUN apt-get update && \
    apt-get install -y autoconf \
      autoconf-archive \
      automake \
      build-essential \
      cmake \
      curl \
      git \
      imagemagick \
      jq \
      mingw-w64 \
      ninja-build \
      pkg-config \
      python3 \
      python3-jinja2 \
      tar \
      tree \
      unzip \
      zip

# needed by vcpkg package
RUN cd /tmp \
  && curl -L -o powershell.deb https://github.com/PowerShell/PowerShell/releases/download/v7.4.12/powershell_7.4.12-1.deb_amd64.deb \
  && dpkg -i powershell.deb || apt-get install -y -f

ARG UID=1000
ARG GID=1000

RUN usermod -u ${UID} ubuntu \
  && groupmod -g ${GID} ubuntu \
  && mkdir /build \
  && chown ubuntu:ubuntu /build

WORKDIR /build
USER ubuntu

ENV CMAKE_MAKE_PROGRAM="ninja"
ENV CMAKE_FIND_ROOT_PATH="/tmp/vcpkg_installed/x64-mingw-dynamic"
ENV CMAKE_TOOLCHAIN_FILE="cmake/x86_64-mingw-w64-toolchain.cmake"
ENV VC_CXX_FLAGS="-w;-Wwrite-strings;-Werror=write-strings"

# run build on container start
ENTRYPOINT "/usr/bin/bash" "-c" "\
  set -eExuo pipefail \
  && ./scripts/bin/vcpkg --init \
  && ./scripts/bin/vcpkg install --triplet=x64-mingw-dynamic --x-install-root=/tmp/vcpkg_installed \
  && jq 'del((.configurePresets[] | select(.name == \"release\")).cacheVariables[\"CMAKE_TOOLCHAIN_FILE\"])' CMakePresets.json > /tmp/CMakePresets1.json \
  && mv /tmp/CMakePresets1.json CMakePresets.json \
  && cmake --workflow --fresh --preset release \
  && tree build/release/RelWithDebInfo \
"
