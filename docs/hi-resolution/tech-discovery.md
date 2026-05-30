This doc details the insights gained on how the video system works in the C&C engine. The aim is to define how we can 
implement a higher resolution than 640x400.

> This was carried out against Tiberian Dawn, but Red Alert works in a similar wayTiberian Dawn

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
