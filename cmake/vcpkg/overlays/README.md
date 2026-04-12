- This directory contains overlays for vcpkg packages
- Overlays allow overriding public packages with modified versions for working around issues
- They also allow falling back to system package manager libraries (`-dev` packages on Ubuntu for example)
  - Again this is useful for fixing issues with packages (e.x. broken download mirrors)
  - See: https://devblogs.microsoft.com/cppblog/using-system-package-manager-dependencies-with-vcpkg/

## Overlays

- `pipewire` - There is a bug fix that hasn't been backported to the 1.4 branch (the current version used by vcpkg), so we just install the latest 1.6 release
  - This should be reverted once pipewire repo backports the fix, as many linux distro's use the older version
  - A better fix would be update our overlay to apply git patches to make 1.4 build correctly
  - Repo issue can be found [here](https://gitlab.freedesktop.org/pipewire/pipewire/-/issues/5141)
