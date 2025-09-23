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

# note: jq/tree/wget/zstd are utilities for this Dockerfile only
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
      g++-mingw-w64-x86-64 \
      ninja-build \
      pkg-config \
      python3 \
      python3-jinja2 \
      tar \
      tree \
      unzip \
      wget \
      zip \
      zstd

# BUG: Needs https://github.com/HolyBlackCat/quasi-msys2 instead
# substitute for vcpkg packages
RUN wget -q -O - https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-libiconv-1.18-1-any.pkg.tar.zst | tar --zstd -x -C / \
  && wget -q -O - https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-vulkan-loader-1~1.4.321.0-1-any.pkg.tar.zst | tar --zstd -x -C / \
  && wget -q -O - https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-SDL2-2.32.10-1-any.pkg.tar.zst | tar --zstd -x -C / \
  && wget -q -O - https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-zlib-1.3.1-1-any.pkg.tar.zst | tar --zstd -x -C / \
  && wget -q -O - https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-libmysofa-1.3.1-1-any.pkg.tar.zst | tar --zstd -x -C / \
  && wget -q -O - https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-openal-1.24.3-2-any.pkg.tar.zst | tar --zstd -x -C / \
  && wget -q -O - https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-fmt-11.2.0-3-any.pkg.tar.zst | tar --zstd -x -C / \
  && wget -q -O - https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-spdlog-1.15.3-1-any.pkg.tar.zst | tar --zstd -x -C / \
  && wget -q -O - https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-lua-5.4.8-1-any.pkg.tar.zst | tar --zstd -x -C /

RUN cd /tmp \
  && wget -q -O LuaBridge-2.8.tar.gz https://github.com/vinniefalco/LuaBridge/archive/refs/tags/2.8.tar.gz \
  && tar -xf LuaBridge-2.8.tar.gz \
  && mv LuaBridge-2.8/Source/LuaBridge /mingw64/include

ARG UID=1000
ARG GID=1000

RUN usermod -u ${UID} ubuntu \
  && groupmod -g ${GID} ubuntu \
  && mkdir /build \
  && chown ubuntu:ubuntu /build

WORKDIR /build
USER ubuntu

ENV CMAKE_MAKE_PROGRAM="ninja"
ENV CMAKE_FIND_ROOT_PATH="/mingw64"
ENV CMAKE_TOOLCHAIN_FILE="cmake/x86_64-mingw-w64-toolchain.cmake"
ENV VC_CXX_FLAGS="-w;-Wwrite-strings;-Werror=write-strings"

# run build on container start
ENTRYPOINT "/usr/bin/bash" "-c" "\
  set -eExuo pipefail \
  && jq 'del((.configurePresets[] | select(.name == \"release\")).cacheVariables[\"CMAKE_TOOLCHAIN_FILE\"])' CMakePresets.json > /tmp/CMakePresets.json \
  && mv -f /tmp/CMakePresets.json CMakePresets.json \
  && cmake --workflow --fresh --preset nco \
  && tree build/nco/RelWithDebInfo \
"
