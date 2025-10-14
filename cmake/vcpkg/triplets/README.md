- This directory contains custom vcpkg triplets
- Triplets are used to control build behaviour for vcpkg packages

## Triplets

- `uni-osx14.cmake` Virtual triplet to allow using previously build binaries created by installing 64 and arm64 vcpkg triplets separately and merging them together using `lipomerge`
  - Requires `VCPKG_MANIFEST_INSTALL` to be set to `OFF` in cmake variables
  - Sets build target to macOS 14.0
- `arm64-osx14.cmake` and `x64-osx14.cmake` are copies of the normal macOS triplets, locked to a build target of macOS 14.0
