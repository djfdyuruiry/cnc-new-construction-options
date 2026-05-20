# High Resolution Implementation Plan

This plan describes the transition from a simple scaling-factor approach to a hybrid system that supports true high-resolution rendering for the map and a scaled, centered approach for menus and FMVs.

## Phase 1: Foundation & Dynamic Resolution
- **Dynamic Resolution Setup:** Modify `GraphicBufferClass` and `VideoSurface` to initialize resolution based on the SDL2 window size instead of fixed constants.
- **Resolution State Management:** Implement a state system to track the current mode:
    - `MODE_HIGH_RES`: Active gameplay (true high-res).
    - `MODE_SCALED`: Menus/FMVs (base resolution scaled to fit).
    - `MODE_DOS`: Scaling from 320x200.
- **Coordinate Mapping:** Update `GraphicViewPortClass` to handle the translation between logical game coordinates and the dynamic screen resolution.

Changes made:
1. Resolution State Management:
- Added ResolutionMode enum (MODE_HIGH_RES, MODE_SCALED, MODE_DOS) to common/video.h.
- Added a global CurrentResolutionMode in common/video_sdl2.cpp, defaulting to MODE_HIGH_RES.
2. Dynamic Resolution Setup:
- Modified Set_Video_Mode in common/video_sdl2.cpp to update the global ScreenWidth and ScreenHeight based on the actual SDL2 window size after creation.
- Updated tiberiandawn/startup.cpp to use the updated ScreenWidth and ScreenHeight when attaching SeenBuff and HidPage, ensuring they match the window resolution instead of using fixed constants.

## Phase 2: In-Game Map & Rendering
- **True High-Res Rendering:** Update the map rendering pipeline to utilize the full dynamic resolution.
- **Buffer Clearing:** Implement a black-fill for areas beyond the map boundaries to remove visual artifacts.
- **Dynamic Viewport:** Ensure the map camera and viewport adapt to the current window aspect ratio.

## Phase la: UI, Menus & FMVs
- **Centering Logic:** Refactor menu and dialog placement to be relative to the screen center rather than fixed offsets.
- **The "Zoom" Implementation:** Implement logic to render menus/FMVs at a base resolution (640x400) and scale them to the center of the window, preserving the aspect ratio.
- **Asset Scaling:** Maintain the existing resolution factor for UI control placement to keep assets crisp while relocating them.

## Phase 4: Sidebar Overhaul
- **Height Adaptation:** Modify the sidebar to occupy 100% of the screen height.
- **Background Tiling:** Implement tiling/stretching for sidebar background assets to fill the vertical space.
- **Power Meter Scaling:** Update the power meter to scale proportionally to the screen height.

## Phase 5: DOS Mode & Final Integration
- **DOS Scaling:** Handle 320x200 DOS mode as a scaled version of the base resolution.
- **Config Options:** Implement a configuration setting to choose between dynamic resolution and a doubling strategy (e.g., 1280x800).
- **Verification:** Validate across multiple aspect ratios (4:3, 16:9, 21:9).
