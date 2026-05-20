This doc details the insights gained on how the video system works in the NCO engine. The aim is to define how we can 
implement a higher resolution than 640x400.

## SDL

- `common/video_sdl2.cpp` implements the low-level integration with SDL2, so the game can draw to the screen
  - `VideoSurfaceSDL2` implements the `VideoSurface` class, which is referenced by game engine code
  - It handles boxing (aspect ratio), scaling and the mouse cursor

## Graphics Buffer

- `GraphicBufferClass` wraps around this using the `VideoSurfacePtr` global pointer
  - This class is responsible for setting the resolution of the game
  - Changing the `::Init` parameters allows changing the resolution
  - The `ScreenWidth` and `ScreenHeight` globals are passed to this by the TD source code
  - These globals are set using the `GBUFF_INIT_WIDTH` and `GBUFF_INIT_HEIGHT` constants

## Game Code

- The resolution is always set to the GBUFF constants, but for DOS-mode the engine sets the view port to 320x200
- Most game code checks a `GraphicViewPortClass` instance to check the current resolution, then sets a resolution factor
- The factor is either `1` for DOS or `2` for C&C 95, this is used in UI control placement etc.
- No assets are scaled, placement of controls and text on the screen is scaled with alternate assets for DOS mode

---

## Hi-Res: Implementation Plan

- Main consideration is whether to resize or just relocate UI elements
  - Menus should just be relocated so they always render in the center of the screen
  - Controls shouldn't be resized (the resolution factor logic can stay in place)
- Only the in-game map screen really benefits from higher resolutions, so other views should be scaled
  - Add logic to 'zoom' into a default resolution (`640x400`) that can activated when game loads
  - It should be deactivated when a map view loads at the start of a scenario
  - This is primarily for FMVs and menus
  - SDL 2 integration code can be updated to handle this with `VideoSurface` changes to add api for setting zoom state
  - Mouse scaling will need handled as in this `zoomed` approach mouse may go off visible surface but still be in-game
- The sidebar for construction and radar needs the most work
  - It should be resized so it fits the entire height of the screen
  - The background should be tiled/stretched so it covers the height
  - Keep 4 items visible for now
      - But a future goal would be to decouple the visibility etc. from the sidebar and and have this be dynamic
      - Current save game logic is hard coupled to the number of visible items etc.
  - Power meter needs to scale to entire height
- Areas of the map that are normally non-visible should be filled in black
  - Currently, then game just fills it with garbage from nearby graphics
  - At any resolution higher than `640x200` the starting missions of both campaigns have very small maps
- DOS mode should just scale from `320x200` rather than attempting hi-res support
