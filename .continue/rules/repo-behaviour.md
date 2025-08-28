---
description: Repo Behaviour
---

This rule provides behaviour guidelines for interacting with this code repository:

- Contains a C++ desktop game
  - `tiberiandawn` directory is the source for `Command & Conquer: Tiberian Dawn`
  - `redalert` directory is the source for `Command & Conquer: Red Alert`
  - `common` directory is source for a shared library, used by both games
- Contains a mix of Legacy C++ code and modern C++ code
- Targets the C++ 2023 Standard
- Targets Linux, Windows and MacOSX
- Supports MSVC, GCC and clang
- Uses SDL2 for Video and Input
- Uses OpenAL for Audio
- Use CMake build orchestration
- Uses vcpkg for package management (with CMake integration)
- Has a markdown wiki in the `wiki` directory
- Local development uses clang
- Uses GitHub actions for CI builds
- Has local development helper scripts in the `scripts` directory
