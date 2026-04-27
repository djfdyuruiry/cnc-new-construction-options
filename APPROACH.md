# Dynamic Resolution Refactoring Approach

## Goal
Increase the internal rendering resolution of the game from a fixed 640x400 to a dynamic or higher resolution, ensuring that UI elements and viewports resize proportionally based on a logical 320x200 base.

## Strategy
The core strategy is to move away from a binary "factor" system (where resolution was either 320x200 or 640x400) and replace hardcoded dimensions with dynamic scaling helpers.

### Key Changes
1. **Scaling Helpers**: Use `S_X(int x)` and `S_Y(int y)` to calculate coordinates and dimensions. These helpers multiply the logical 320x200 coordinates by the current `ScreenScale`.
2. **Removing the "Factor" System**: 
   - Replace calls to `Get_Resolution_Factor()` with checks against `ScreenWidth` (e.g., `ScreenWidth > BASE_WIDTH`).
   - Remove bit-shifting operations (e.g., `<< factor`) that were used to double dimensions for high-resolution mode.
3. **Dynamic Buffer Scaling**: Update `SysMemPage.Scale` calls to use `ScreenWidth` and `ScreenHeight` instead of hardcoded 640x398 values to ensure full-screen coverage at any internal resolution.
4. **UI Layout Refactoring**: Update menu, sidebar, and dialog layouts to use scaled values, ensuring proportional placement.

## Affected Files
- `tiberiandawn/menus.cpp`: Refactored menu layouts and dimensions.
- `tiberiandawn/sidebar.cpp`: Removed bit-shifting, updated button positions and strip dimensions.
- `tiberiandawn/dialog.cpp`: Replaced resolution factor checks with `ScreenWidth` comparisons.
- `tiberiandawn/init.cpp`: Updated internal buffer scaling for intro/teaser sequences.
- `tiberiandawn/ending.cpp`: Updated internal buffer scaling for ending sequences.
