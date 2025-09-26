##
# This Dockerfile is used to test a clean environment build, ensuring that
# all build dependencies are documented and understood. 
#
# - C++ toolchain used it clang
# - Mirrors CI build defined in .github/workflows/linux.yml
# - See wiki page: wiki/5.Building.md
##
FROM ubuntu:24.04

ENV LANG="C.UTF-8"

# note: tree is a utility for this Dockerfile only
RUN apt-get update && \
    apt-get install -y autoconf \
      autoconf-archive \
      automake \
      build-essential \
      cmake \
      clang \
      curl \
      git \
      imagemagick \
      ninja-build \
      pkg-config \
      python3 \
      python3-jinja2 \
      tar \
      tree \
      unzip \
      zip

# vcpkg package: SDL2 (X11 & Wayland support)
RUN apt-get install -y \
      libx11-dev \
      libxft-dev \
      libxext-dev \
      libwayland-dev \
      libxkbcommon-dev \
      libegl1-mesa-dev

# vcpkg package: OpenAL (pipewire support)
RUN apt-get install -y gettext

# vcpkg package: libxcrypt
RUN apt-get install -y libltdl-dev

ARG UID=1000
ARG GID=1000

RUN usermod -u ${UID} ubuntu \
  && groupmod -g ${GID} ubuntu \
  && mkdir /build \
  && chown ubuntu:ubuntu /build

WORKDIR /build
USER ubuntu

ENV CC="clang"
ENV CXX="clang++"
ENV CMAKE_MAKE_PROGRAM="ninja"
ENV VC_CXX_FLAGS="-w;-Wwrite-strings;-Werror=write-strings"

# run build on container start
ENTRYPOINT "/usr/bin/bash" "-c" "\
  set -eEuo pipefail \
  && ./scripts/bin/vcpkg --init \
  && cmake --workflow --preset nco \
  && tree build/nco/RelWithDebInfo \
"
