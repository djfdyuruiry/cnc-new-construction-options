# Hi-Resolution Support Documentation

This is the complete documentation for high resolution support added to the NCO Command & Conquer engine, allowing running Tiberian Dawn at resolutions higher than 640x400.

## Background

- The original DOS C&C ran at 320x200 (and the DOS version of Red Alert)
- C&C Gold and Red Alert 95 ran at double that, 640x400 stretched to 640x480
- Movies, animations and menu background are loaded in at 640x400 
- Most assets are either 640x400 or interpolated/scaled 320x200 (movies, animations etc.)

## Video Backend

> Hi-resolution support targets the SDL2 video backend only, DirectX and SDL1 builds just have a stub implementation that does nothing.

- SDL2 backend of Vanilla Conquer either rendered DOS mode (320x200) or the default resolution, it then scaled this up to match the INI config width and height or defaulted to fullscreen, using the current desktop resolution
- This has been updated to accept any in-game resolution, but still scale that resolution to fullscreen borderless or a standalone window
- `StretchWidth` and `StretchHeight` settings have been added to allow setting the scaled resolution for windowed/fullscreen 
- Functions added to enter/leave a 'zoom' mode allow cropping output to 640x400 and scaling that up, to support screens with fixed layouts and graphics

See: [video_sdl2.cpp](../../common/video_sdl2.cpp) and [video.h](../../common/video.h)

### Resolution Modes

Internally the SDL2 backend uses an enum to set the 'mode' which drives how then window is initialized and resolution configured:

- The game engine can set the mode to `MODE_HIGH_RES`, this enables hi-res and 'zooming' in and out of 640x400
- `MODE_ZOOM` mode is used to signal that SDL2 should crop the game to this resolution when rendering frames
- The `Enter_Zoomed_Resolution_Mode` and `Leave_Zoomed_Resolution_Mode` functions trigger the change in mode
- For code that needs to determine the current displayed ares (Zoomed or full) the `Try_Get_Resolution_Mode_Width` and `Try_Get_Resolution_Mode_Height` return either the zoomed resolution or full resolution dependent on the current mode

See: `ResolutionMode` in [video.h](../../common/video.h)

## Tiberian Dawn

Resolution settings are stored in `CONQUER.INI` and loaded on startup, the `Resolve_Resolution_Mode` function in [startup.cpp](../../tiberiandawn/startup.cpp) determines the mode to use.

Only the scenario view is true hi-res, that is the in-game screen with the map and sidebar. Briefings and dialog
boxes (Options menu etc.) in-game are also hi-res.

The below diagram shows how the video backend is used from Tiberian Dawn:

![Common Library Video Calls](common-lib.drawio.png)

### Scenario View

- The sidebar logic was reworked to use relative co-ordinates, anchoring all controls to the top-right hand side of the screen
  - Sidebar resolution is unchanged, but an additional texture is drawn below it to fill the empty space
- Maps that are smaller than the current resolution can cause some problems but several changes were added to mitigate this:
  - All cells and objects outside the intended map size (set in scenario INI) are never rendered
  - Black rectangles are added on sides of the map to prevent artifacts from units at the edges of the map (which may be right of the map or below the map or both)
  - When a scenario like this is started/restarted or loaded from a save, the map view dimensions are refreshed to ensure it is displayed correctly before render
- Save/load functionality was updated to ensure that the `TabClass::One_Time` method and base methods are called to refresh resolution dependant fields

### CPS Images

At various points static graphics in CPS format are rendered to the screen, internally C&C scales these up from the original DOS resolution to 640x400. This has been updated to scale them up to whatever the current in-game resolution is.

Interactive menus using CPS images are still 'zoomed' in to 640x400.

## Known issues

- Need to review FROM_JSON methods to ensure that any calculated constants or variables tied to game resolution are refreshed on save load (calling One_Time with a flag to partially reset object)
  - I have added ` // TODO: Remove and test, as it is calculated from resolution` to fields of interest
  - For now, the `TabClass::One_Time` call in `FROM_JSON` functions should reset any incorrect field values
