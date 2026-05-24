## Resolution Modes

A mode system will be implemented to compliment the existing video subsystem, aiming to only target the SDL2 backend.

- `SCALED` - current mode, where the game always runs at 640x400 internally but scales to the native fullscreen resolution or has a stretch resolution for a window
- `DOS` - existing mode for DOS graphics, same as `SCALED` but uses 300x200
- `HI_RES` - new mode, that will support internal resolutions above 640x400. Can scale to a native fullscreen/stretch resolution for window
- `ZOOM` - temporary mode that can be entered at runtime to tell backend to zoom to 640x400 (for graphics that are fixed to 640x400 - videos, score screens etc.)

### Initialisation

The game engine will read the below config from `CONQUER.INI` to determine the correct resolution mode:

- `Width`/`Height` - internal resolution
- `StretchWidth`/`StretchHeight` - window/fullscreen resolution, renamed from `WindowWidth`/`WindowHeight`. If set to 0, native fullscreen will be used or window will be the same size as the internal resolution
- `DOSMode` - use DOS graphics at a fixed 320x200 internal resolution

The mode is then mapped based on:

- `DOSMode` -> `DOS`
- `Width`/`Height` < 640x400 -> `SCALED` (Forces resolution to 640x400)
- `Width`/`Height` == 640x400 -> `SCALED`
- `Width`/`Height` > 640x400 -> `HI_RES`

### Zooming

At several points in the game logic we will ensure that the resolution mode is set to `ZOOM`. We will update the SDL2 
backend to change the area that is rendered to the output, cropping it to 640x400 to effectively 'zoom' in on the 
original screens and graphics.

This covers:

- Videos (already interpolated from 320x200 to 640x400)
- Main Menu
- Game Score Screen
- Interactive videos (CPS format - GDI/NOD selection screen for example)

When entering a scenario the game will 'zoom' out to show the entire internal resolution that can utilized by the map 
and sidebar.

### Scenario View

The main view of the map and sidebar will have several changes made to support high resolution mode:

- Sidebar will remain at the default width, with the background tiled to fill the entire height
- Power meter on sidebar will fill the entire height with tiling graphics as needed
- Credits tab and sidebar buttons will be 'anchored' to the sidebar location, making placement relative to that control
- Graphics outside the current map width/height will not be rendered (they are still in the game engine state, just not visible)

---

Known issues:

- Small maps 'snaps' the view after moving the mouse for the first time (e.x. first GDI mission is smaller than even 800x600 resolution)
  - Added a sort-of 'fix' for this, which is just to force the snap-in if map is detected to be smaller than current resolution
- Need to review FROM_JSON methods to ensure that any calculated constants or variables tied to game resolution are refreshed on save load (calling One_Time with a flag to partially reset object)
  - I have added ` // TODO: Remove and test, as it is calculated from resolution` to fields of interest
