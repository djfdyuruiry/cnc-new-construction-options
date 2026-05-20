# High Resolution Points of Interest

This document summarizes the technical findings and the implementation plan for high resolution support in the game.

## Key Files and Components

- `common/video_sdl2.cpp`: Implements `VideoSurfaceSDL2`, handling SDL2 integration, boxing, scaling, and the mouse cursor.
- `GraphicBufferClass`: Responsible for initializing game resolution and managing `ScreenWidth` and `ScreenHeight` globals.
- `GraphicViewPortClass`: Used by game code to determine current resolution and apply scaling factors (1 for DOS mode, 2 for C&C 95).

## Implementation Status

The project is currently in **Phase 1: Foundation & Dynamic Resolution**.

### Completed in Phase 1
- **Resolution State Management**: Introduced `ResolutionMode` enum (`MODE_HIGH_RES`, `MODE_SCALED`, `MODE_DOS`) and a global `CurrentResolutionMode`.
- **Dynamic Resolution**: `Set_Video_Mode` now updates `ScreenWidth` and `ScreenHeight` based on the actual SDL2 window size.
- **Initialization**: `tiberiandawn/startup.cpp` now ensures `SeenBuff` and `HidPage` match the window resolution.

## Future Roadmap

### Phase 2: In-Game Map & Rendering
- Implement true high-res rendering for the map.
- Implement black-fill for areas beyond map boundaries to prevent visual artifacts.
- Adapt the map camera and viewport to the window aspect ratio.

### Phase 3: UI, Menus & FMVs
- Move menu and dialog placement to be relative to the screen center.
- Implement a "Zoom" mode: render at 640x400 and scale to the center of the window.

### Phase 4: Sidebar Overhaul
- Expand the sidebar to occupy 100% of the screen height.
 bentuk Tiling/stretching for sidebar background assets.
- Scale the power meter to fit the screen height.

### Phase 5: DOS Mode & Final Integration
- Implement scaling for 320x200 DOS mode.
- Add configuration options for resolution strategies.
- Validate across various aspect ratios (4:3, 16:9, 21:9).
