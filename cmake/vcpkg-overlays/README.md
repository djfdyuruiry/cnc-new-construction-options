- This directory contains overlays for vcpkg packages
- Overlays allow overriding public packages with modified versions for working around issues
- They also allow falling back to system package manager libraries (`-dev` packages on Ubuntu for example)
  - Again this is useful for fixing issues with packages (e.x. broken download mirrors)
  - See: https://devblogs.microsoft.com/cppblog/using-system-package-manager-dependencies-with-vcpkg/

## Overlays

- **gettext**
  - Sub-dependency of `pipewire` package (used by OpenAL on Linux)
  - Empty virtual package to allow using system `gettext` instead
  - Download link in the public package is broken, see: https://github.com/microsoft/vcpkg/issues/47462
