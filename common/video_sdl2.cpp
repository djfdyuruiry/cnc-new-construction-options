//
// Copyright 2020 Electronic Arts Inc.
//
// TiberianDawn.DLL and RedAlert.dll and corresponding source code is free
// software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.

// TiberianDawn.DLL and RedAlert.dll and corresponding source code is distributed
// in the hope that it will be useful, but with permitted additional restrictions
// under Section 7 of the GPL. See the GNU General Public License in LICENSE.TXT
// distributed with this program. You should have received a copy of the
// GNU General Public License along with permitted additional restrictions
// with this program. If not, see https://github.com/electronicarts/CnC_Remastered_Collection

/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D   A S S O C I A T E S   **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Westwood Win32 Library                   *
 *                                                                         *
 *                    File Name : DDRAW.CPP                                *
 *                                                                         *
 *                   Programmer : Philip W. Gorrow                         *
 *                                                                         *
 *                   Start Date : October 10, 1995                         *
 *                                                                         *
 *                  Last Update : October 10, 1995   []                    *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*=========================================================================*/
/* The following PRIVATE functions are in this file:                       */
/*=========================================================================*/

/*= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/

#include "gbuffer.h"
#include "palette.h"
#include "video.h"
#include "wwkeyboard.h"
#include "wwmouse.h"
#include "settings.h"
#include "debugstring.h"
#include "logger.h"

#include <SDL.h>
#include <SDL2/SDL_image.h>

// C&C 95 Resolution
static constexpr auto DefaultWidth = 640;
static constexpr auto DefaultHeight = 400;
// C&C DOS Resolution
static constexpr auto DefaultDosWidth = DefaultWidth / 2;
static constexpr auto DefaultDosHeight = DefaultHeight / 2;

extern WWKeyboardClass* Keyboard;
static SDL_Window* window;
static SDL_Renderer* renderer;
static SDL_Palette* palette;
static Uint32 pixel_format;
static SDL_Rect render_dst;
static ResolutionMode CurrentResolutionMode = MODE_DEFAULT;
static bool StretchOriginalResolution = true;

static struct
{
    int GameW;
    int GameH;
    bool Clip;
    float ScaleX{1.0f};
    float ScaleY{1.0f};
    void* Raw;
    int W;
    int H;
    int HotX;
    int HotY;
    float X;
    float Y;
    SDL_Cursor* Pending;
    SDL_Cursor* Current;
    SDL_Surface* Surface;
} hwcursor;

#define ARRAY_SIZE(x) int(sizeof(x) / sizeof(x[0]))
#define MAKEFORMAT(f)                                                                                                  \
    {                                                                                                                  \
        SDL_PIXELFORMAT_##f, #f                                                                                        \
    }
Uint32 SettingsPixelFormat()
{
    /*
    ** Known good RGB formats for both the surface and texture.
    */
    static struct
    {
        Uint32 format;
        std::string name;
    } formats[] = {
        MAKEFORMAT(ARGB8888),
        MAKEFORMAT(RGBA8888),
        MAKEFORMAT(ABGR8888),
        MAKEFORMAT(BGRA8888),
        MAKEFORMAT(RGB24),
        MAKEFORMAT(BGR24),
        MAKEFORMAT(RGB888),
        MAKEFORMAT(BGR888),
        MAKEFORMAT(RGB555),
        MAKEFORMAT(BGR555),
        MAKEFORMAT(RGB565),
        MAKEFORMAT(BGR565),
    };

    std::string str = Settings.Video.PixelFormat;

    for (auto& c : str) {
        c = toupper(c);
    }

    for (int i = 0; i < ARRAY_SIZE(formats); i++) {
        if (str.compare(formats[i].name) == 0) {
            return formats[i].format;
        }
    }

    return SDL_PIXELFORMAT_UNKNOWN;
}

static void Update_HWCursor();

static void Update_HWCursor_Settings()
{
    /*
    ** Update mouse scaling settings.
    */
    int win_w, win_h;
    SDL_GetRendererOutputSize(renderer, &win_w, &win_h);

    const auto resolution_mode = Get_Current_Resolution_Mode();

    if (resolution_mode == MODE_HIGH_RES) {
        hwcursor.GameW = Settings.Video.Width;
        hwcursor.GameH = Settings.Video.Height;
    } else if (resolution_mode == MODE_DEFAULT) {
        hwcursor.GameW = DefaultWidth;
        hwcursor.GameH = DefaultHeight;
    } else if (resolution_mode == MODE_DOS) {
        hwcursor.GameW = DefaultDosWidth;
        hwcursor.GameH = DefaultDosHeight;
    }

    hwcursor.ScaleX = win_w / (float)hwcursor.GameW;
    hwcursor.ScaleY = win_h / (float)hwcursor.GameH;

    /*
    ** Update screen boxing settings.
    */
    float ar = (float)hwcursor.GameW / hwcursor.GameH;
    if (Settings.Video.Boxing) {
        size_t colonPos = Settings.Video.BoxingAspectRatio.find(":");
        std::string arW;
        std::string arH;

        /*
        ** If we don't have a valid string for aspect ratio, default back to 4:3.
        */
        if (colonPos == std::string::npos) {
            arW = "4";
            arH = "3";
        } else {
            size_t arLen = Settings.Video.BoxingAspectRatio.length();
            arW = Settings.Video.BoxingAspectRatio.substr(0, colonPos);
            arH = Settings.Video.BoxingAspectRatio.substr(colonPos + 1, arLen - colonPos);
        }

        ar = std::stof(arW) / std::stof(arH);

        render_dst.w = win_w;
        render_dst.h = render_dst.w / ar;
        if (render_dst.h > win_h) {
            render_dst.h = win_h;
            render_dst.w = render_dst.h * ar;
        }
        render_dst.x = (win_w - render_dst.w) / 2;
        render_dst.y = (win_h - render_dst.h) / 2;
    } else {
        render_dst.w = win_w;
        render_dst.h = win_h;
        render_dst.x = 0;
        render_dst.y = 0;
    }

    /*
    ** Ensure cursor clip is in the desired state.
    */
    Set_Video_Cursor_Clip(hwcursor.Clip);

    /*
    ** Update visible cursor scaling.
    */
    Update_HWCursor();
}

class SurfaceMonitorClassDummy : public SurfaceMonitorClass
{

public:
    SurfaceMonitorClassDummy()
    {
    }

    virtual void Restore_Surfaces()
    {
    }

    virtual void Set_Surface_Focus(bool in_focus)
    {
    }

    virtual void Release()
    {
    }
};

SurfaceMonitorClassDummy AllSurfacesDummy;           // List of all direct draw surfaces
SurfaceMonitorClass& AllSurfaces = AllSurfacesDummy; // List of all direct draw surfaces

/***********************************************************************************************
 * Set_Video_Mode -- Initializes Direct Draw and sets the required Video Mode                  *
 *                                                                                             *
 * INPUT:           int width           - the width of the video mode in pixels                *
 *                  int height          - the height of the video mode in pixels               *
 *                  int bits_per_pixel  - the number of bits per pixel the video mode supports *
 *                                                                                             *
 * OUTPUT:     none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/26/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
bool Set_Video_Mode(int& w, int& h, int bits_per_pixel)
{
    if (Settings.Video.VideoDriver != "default") {
        CNC_LOG_INFO("Using SDL video driver hint: {}", Settings.Video.VideoDriver);
        SDL_SetHint(SDL_HINT_VIDEODRIVER, Settings.Video.VideoDriver.c_str());
    } else {
#ifdef __linux__
        // Workaround for XWayland on linux returning all displays as one massive render surface; image under XWayland
        // can be zoomed in (due to a large reported resolution) and always renders on left most monitor.
        //
        // Note: SDL2 video driver auto selection prefers x11 over wayland, so XWayland is automatically selected under
        // wayland environments. SDL3 prefers wayland over x11, so this can be removed if library is upgraded.
        if (std::getenv("WAYLAND_DISPLAY") != nullptr) {
            CNC_LOG_INFO("Environment variable 'WAYLAND_DISPLAY' found, using SDL video driver hint: wayland");
            SDL_SetHint(SDL_HINT_VIDEODRIVER, "wayland");
        }
#else
        CNC_LOG_DEBUG("No default video driver override logic for this platform");
#endif
    }

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_ShowCursor(SDL_DISABLE);

    // dump info on detected displays
    auto num_displays = SDL_GetNumVideoDisplays();

    CNC_LOG_INFO("Found {} display(s)", num_displays);
    for (auto i = 0; i < num_displays; i++)
    {
        SDL_DisplayMode display_mode;
        SDL_GetCurrentDisplayMode(i, &display_mode);

        if (display_mode.refresh_rate == 0) {
            CNC_LOG_INFO("Display #{} mode: {}x{}@?hz", i + 1, display_mode.w, display_mode.h);
        } else {
            CNC_LOG_INFO(
                "Display #{} mode: {}x{}@{}hz", i + 1, display_mode.w, display_mode.h, display_mode.refresh_rate
            );
        }
    }

    const auto display = Settings.Video.Display - 1; // SDL displays are 0 indexed
    int x = SDL_WINDOWPOS_CENTERED_DISPLAY(display);
    int y = SDL_WINDOWPOS_CENTERED_DISPLAY(display);
    int win_w = w;
    int win_h = h;
    int win_flags = 0;
    Uint32 requested_pixel_format = SettingsPixelFormat();

    if (!Settings.Video.Windowed) {
        /*
        ** Native fullscreen if no proper width and height set.
        */
        if (Settings.Video.StretchWidth < w || Settings.Video.StretchHeight < h) {
            win_w = Settings.Video.StretchWidth = 0;
            win_h = Settings.Video.StretchHeight = 0;
            win_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        } else {
            win_w = Settings.Video.StretchWidth;
            win_h = Settings.Video.StretchHeight;
            win_flags |= SDL_WINDOW_FULLSCREEN;
        }

        x = SDL_WINDOWPOS_UNDEFINED_DISPLAY(display);
        y = SDL_WINDOWPOS_UNDEFINED_DISPLAY(display);
    } else if (Settings.Video.StretchWidth > w || Settings.Video.StretchHeight > h) {
        win_w = Settings.Video.StretchWidth;
        win_h = Settings.Video.StretchHeight;
    } else {
        Settings.Video.Width = win_w;
        Settings.Video.Height = win_h;
    }

    window = SDL_CreateWindow("CNC: New Construction Options", x, y, win_w, win_h, win_flags);

    if (window == nullptr) {
        DBG_ERROR("SDL_CreateWindow failed: %s", SDL_GetError());
        Reset_Video_Mode();
        return false;
    }

    SDL_GetWindowSize(window, &win_w, &win_h);

    w = Settings.Video.Width;
    h = Settings.Video.Height;

    DBG_INFO("Created SDL2 %s window in %dx%d", (win_flags ? "fullscreen" : "windowed"), win_w, win_h);


    pixel_format = SDL_GetWindowPixelFormat(window);
    if (pixel_format == SDL_PIXELFORMAT_UNKNOWN || SDL_BITSPERPIXEL(pixel_format) < 16) {
        DBG_ERROR("SDL2 window pixel format unsupported: %s (%d bpp)",
                  SDL_GetPixelFormatName(pixel_format),
                  SDL_BITSPERPIXEL(pixel_format));
        Reset_Video_Mode();
        return false;
    }

    DBG_INFO("  pixel format: %s (%d bpp)", SDL_GetPixelFormatName(pixel_format), SDL_BITSPERPIXEL(pixel_format));

    DBG_INFO("SDL2 render drivers available: (user preference '%s')", Settings.Video.RenderDriver.c_str());
    int renderer_index = -1;
    for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
        SDL_RendererInfo info;
        if (SDL_GetRenderDriverInfo(i, &info) == 0) {
            if (Settings.Video.RenderDriver.compare(info.name) == 0) {
                renderer_index = i;
            }

            DBG_INFO(" %s%s", info.name, (i == renderer_index ? " (selected)" : ""));
        }
    }

    renderer = SDL_CreateRenderer(window, renderer_index, SDL_RENDERER_TARGETTEXTURE);
    if (renderer == nullptr) {
        DBG_ERROR("SDL_CreateRenderer failed: %s", SDL_GetError());
        Reset_Video_Mode();
        return false;
    }

    SDL_RendererInfo info;
    if (SDL_GetRendererInfo(renderer, &info) != 0) {
        DBG_ERROR("SDL_GetRendererInfo failed: %s", SDL_GetError());
        Reset_Video_Mode();
        return false;
    }

    DBG_INFO("Initialized SDL2 driver '%s'", info.name);
    DBG_INFO("  flags:");
    if (info.flags & SDL_RENDERER_SOFTWARE) {
        DBG_INFO("    SDL_RENDERER_SOFTWARE");
    }
    if (info.flags & SDL_RENDERER_ACCELERATED) {
        DBG_INFO("    SDL_RENDERER_ACCELERATED");
    }
    if (info.flags & SDL_RENDERER_PRESENTVSYNC) {
        DBG_INFO("    SDL_RENDERER_PRESENT_VSYNC");
    }
    if (info.flags & SDL_RENDERER_TARGETTEXTURE) {
        DBG_INFO("    SDL_RENDERER_TARGETTEXTURE");
    }

    DBG_INFO("  max texture size: %dx%d", info.max_texture_width, info.max_texture_height);

    DBG_INFO("  %d texture formats supported: (user preference '%s')",
             info.num_texture_formats,
             SDL_GetPixelFormatName(requested_pixel_format));

    /*
    ** Pick the first pixel format or the user requested one. It better be RGB.
    */
    pixel_format = SDL_PIXELFORMAT_UNKNOWN;
    for (int i = 0; i < info.num_texture_formats; i++) {
        if ((pixel_format == SDL_PIXELFORMAT_UNKNOWN && i == 0) || info.texture_formats[i] == requested_pixel_format) {
            pixel_format = info.texture_formats[i];
        }
    }

    for (int i = 0; i < info.num_texture_formats; i++) {
        DBG_INFO("    %s%s",
                 SDL_GetPixelFormatName(info.texture_formats[i]),
                 (pixel_format == info.texture_formats[i] ? " (selected)" : ""));
    }

    /*
    ** Set requested scaling algorithm.
    */
    if (!SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, Settings.Video.Scaler.c_str(), SDL_HINT_OVERRIDE)) {
        DBG_WARN("  scaler '%s' is unsupported");
    } else {
        DBG_INFO("  scaler set to '%s'", Settings.Video.Scaler.c_str());
    }

    if (palette == nullptr) {
        palette = SDL_AllocPalette(256);
    }

    /*
    ** Set mouse scaling options.
    */
    Update_HWCursor_Settings();
    hwcursor.X = hwcursor.GameW / 2;
    hwcursor.Y = hwcursor.GameH / 2;

    /*
    ** Init gamepad.
    */
    if (Settings.Mouse.ControllerEnabled) {
        SDL_Init(SDL_INIT_GAMECONTROLLER);
        Keyboard->Open_Controller();
    }

    return true;
}

void Toggle_Video_Fullscreen()
{
    Settings.Video.Windowed = !Settings.Video.Windowed;

    if (!Settings.Video.Windowed) {
        if (Settings.Video.StretchWidth == 0 || Settings.Video.StretchHeight == 0) {
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        } else {
            SDL_SetWindowSize(window, Settings.Video.StretchWidth, Settings.Video.StretchHeight);
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
        }
    } else {
        SDL_SetWindowFullscreen(window, 0);

        if (Settings.Video.StretchWidth == 0 || Settings.Video.StretchHeight == 0) {
            SDL_SetWindowSize(window, Settings.Video.Width, Settings.Video.Height);
        } else {
            SDL_SetWindowSize(window, Settings.Video.StretchWidth, Settings.Video.StretchHeight);
        }
    }

    Update_HWCursor_Settings();
}

void Get_Video_Scale(float& x, float& y)
{
    x = hwcursor.ScaleX;
    y = hwcursor.ScaleY;
}

void Set_Video_Cursor_Clip(bool clipped)
{
    hwcursor.Clip = clipped;

    if (window) {
        int relative;

        if (Settings.Video.Windowed) {
            SDL_SetWindowGrab(window, hwcursor.Clip ? SDL_TRUE : SDL_FALSE);
            relative = SDL_SetRelativeMouseMode(
                Settings.Mouse.ControllerEnabled || (Settings.Mouse.RawInput && hwcursor.Clip) ? SDL_TRUE : SDL_FALSE);

            /*
            ** When grabbing with raw input, move in-game cursor where the real cursor was and vice versa.
            */
            if (Settings.Mouse.RawInput) {
                if (hwcursor.Clip) {
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    hwcursor.X = x / hwcursor.ScaleX;
                    hwcursor.Y = y / hwcursor.ScaleY;
                } else {
                    SDL_WarpMouseInWindow(window, hwcursor.X * hwcursor.ScaleX, hwcursor.Y * hwcursor.ScaleY);
                }
            }
        } else {
            SDL_SetWindowGrab(window, SDL_TRUE);
            relative = SDL_SetRelativeMouseMode(Settings.Mouse.RawInput ? SDL_TRUE : SDL_FALSE);
        }

        if (relative < 0) {
            DBG_ERROR("Raw input not supported, disabling.");
            Settings.Mouse.RawInput = false;
        }
    }
}

void Move_Video_Mouse(const float xrel, const float yrel)
{
    auto next_x = hwcursor.X;
    auto next_y = hwcursor.Y;

    if (Keyboard->Is_Gamepad_Active() || hwcursor.Clip || !Settings.Video.Windowed) {
        next_x += xrel * (Settings.Mouse.Sensitivity / 100.0f);
        next_y += yrel * (Settings.Mouse.Sensitivity / 100.0f);
    }

    if (next_x >= hwcursor.GameW) {
        next_x = hwcursor.GameW - 1;
    } else if (next_x < 0) {
        next_x = 0;
    }

    if (next_y >= hwcursor.GameH) {
        next_y = hwcursor.GameH - 1;
    } else if (next_y < 0) {
        next_y = 0;
    }

    if (
        Get_Current_Resolution_Mode() == MODE_ORIGINAL_RES
        && (next_x > (DefaultWidth - 1) || next_y > (DefaultHeight - 1))
    ) {
        // prevent mouse leaving the bounds of original resolution content
        return;
    }

    hwcursor.X = next_x;
    hwcursor.Y = next_y;
}

void Move_Video_Mouse_Absolute(const int x, const int y)
{
    hwcursor.X = x;
    hwcursor.Y = y;
}

void Get_Video_Mouse(int& x, int& y)
{
    if (Keyboard->Is_Gamepad_Active() || (Settings.Mouse.RawInput && (hwcursor.Clip || !Settings.Video.Windowed))) {
        x = hwcursor.X;
        y = hwcursor.Y;
    } else {
        float scale_x, scale_y;
        Get_Video_Scale(scale_x, scale_y);
        SDL_GetMouseState(&x, &y);
        x /= scale_x;
        y /= scale_y;
    }
}

/***********************************************************************************************
 * Reset_Video_Mode -- Resets video mode and deletes Direct Draw Object                        *
 *                                                                                             *
 * INPUT:		none                                                                            *
 *                                                                                             *
 * OUTPUT:     none                                                                            *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/26/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
void Reset_Video_Mode(void)
{
    if (hwcursor.Pending) {
        SDL_FreeCursor(hwcursor.Pending);
        hwcursor.Pending = nullptr;
    }

    if (hwcursor.Current) {
        SDL_FreeCursor(hwcursor.Current);
        hwcursor.Current = nullptr;
    }

    if (hwcursor.Surface) {
        SDL_FreeSurface(hwcursor.Surface);
        hwcursor.Surface = nullptr;
    }

    SDL_DestroyRenderer(renderer);
    renderer = nullptr;

    SDL_FreePalette(palette);
    palette = nullptr;

    SDL_DestroyWindow(window);
    window = nullptr;

    Keyboard->Close_Controller();
}

static void Update_HWCursor()
{
    float scale_x = 1.0f;
    float scale_y = 1.0f;
    int scaled_w = hwcursor.W;
    int scaled_h = hwcursor.H;

    /*
    ** Pre-scale cursor *only* if we are not emulating a hw cursor.
    */
    if (Settings.Video.HardwareCursor) {
        scale_x = hwcursor.ScaleX;
        scale_y = hwcursor.ScaleY;
        scaled_w *= scale_x;
        scaled_h *= scale_y;
    }

    /*
    ** Allocate or reallocate surface if it has the wrong size.
    */
    if (hwcursor.Surface == nullptr || hwcursor.Surface->w != scaled_w || hwcursor.Surface->h != scaled_h) {
        if (hwcursor.Surface) {
            SDL_FreeSurface(hwcursor.Surface);
        }

        /*
        ** Real HW cursor needs to be scaled up. Emulated can use original cursor data.
        */
        if (Settings.Video.HardwareCursor) {
            hwcursor.Surface = SDL_CreateRGBSurfaceWithFormat(0, scaled_w, scaled_h, 8, SDL_PIXELFORMAT_INDEX8);
        } else {
            hwcursor.Surface =
                SDL_CreateRGBSurfaceFrom(hwcursor.Raw, hwcursor.W, hwcursor.H, 8, hwcursor.W, 0, 0, 0, 0);
        }

        SDL_SetSurfacePalette(hwcursor.Surface, palette);
        SDL_SetColorKey(hwcursor.Surface, SDL_TRUE, 0);
    }

    /*
    ** Prepare HW cursor by scaling up and creating the SDL version.
    */
    if (Settings.Video.HardwareCursor) {
        uint8_t* src = (uint8_t*)hwcursor.Raw;
        uint8_t* dst = (uint8_t*)hwcursor.Surface->pixels;
        int src_pitch = hwcursor.W;
        int dst_pitch = hwcursor.Surface->pitch;

        for (int y = 0; y < scaled_h; y++) {
            for (int x = 0; x < scaled_w; x++) {
                dst[dst_pitch * y + x] = src[src_pitch * (int)(y / scale_y) + (int)(x / scale_x)];
            }
        }

        if (hwcursor.Pending) {
            SDL_FreeCursor(hwcursor.Pending);
        }

        /*
        ** Queue new cursor to be set during frame flip.
        */
        hwcursor.Pending =
            SDL_CreateColorCursor(hwcursor.Surface, hwcursor.HotX * hwcursor.ScaleX, hwcursor.HotY * hwcursor.ScaleY);
    }
}

void Set_Video_Cursor(void* cursor, int w, int h, int hotx, int hoty)
{
    hwcursor.Raw = cursor;
    hwcursor.W = w;
    hwcursor.H = h;
    hwcursor.HotX = hotx;
    hwcursor.HotY = hoty;

    Update_HWCursor();
}

/***********************************************************************************************
 * Get_Free_Video_Memory -- returns amount of free video memory                                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   bytes of available video RAM                                                      *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    11/29/95 12:52PM ST : Created                                                            *
 *=============================================================================================*/
unsigned int Get_Free_Video_Memory(void)
{
    return 1000000000;
}

/***********************************************************************************************
 * Get_Video_Hardware_Caps -- returns bitmask of direct draw video hardware support            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   hardware flags                                                                    *
 *                                                                                             *
 * WARNINGS: Must call Set_Video_Mode 1st to create the direct draw object                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    1/12/96 9:14AM ST : Created                                                              *
 *=============================================================================================*/
unsigned Get_Video_Hardware_Capabilities(void)
{
    return VIDEO_BLITTER;
}

/***********************************************************************************************
 * Wait_Vert_Blank -- Waits for the start (leading edge) of a vertical blank                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
void Wait_Vert_Blank(void)
{
}

/***********************************************************************************************
 * Set_Palette -- set a direct draw palette                                                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    ptr to 768 rgb palette bytes                                                      *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    10/11/95 3:33PM ST : Created                                                             *
 *=============================================================================================*/
void Set_DD_Palette(void* rpalette)
{
    SDL_Color colors[256];

    unsigned char* rcolors = (unsigned char*)rpalette;
    for (int i = 0; i < 256; i++) {
        colors[i].r = (unsigned char)rcolors[i * 3] << 2;
        colors[i].g = (unsigned char)rcolors[i * 3 + 1] << 2;
        colors[i].b = (unsigned char)rcolors[i * 3 + 2] << 2;
        colors[i].a = 0xFF;
    }

    /*
    ** First color is transparent. This needs to be set so that hardware cursor has transparent
    ** surroundings when converting from 8-bit to 32-bit.
    */
    colors[0].a = 0;

    SDL_SetPaletteColors(palette, colors, 0, 256);

    /*
    ** Cursor needs to be updated when palette changes.
    */
    Update_HWCursor();
}

/***********************************************************************************************
 * Wait_Blit -- waits for the DirectDraw blitter to become idle                                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07-25-95 03:53pm ST : Created                                                             *
 *=============================================================================================*/

void Wait_Blit(void)
{
}

/***********************************************************************************************
 * SMC::SurfaceMonitorClass -- constructor for surface monitor class                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    11/3/95 3:23PM ST : Created                                                              *
 *=============================================================================================*/

SurfaceMonitorClass::SurfaceMonitorClass()
{
    SurfacesRestored = false;
}

/*
** VideoSurfaceDDraw
*/
class VideoSurfaceSDL2;
static VideoSurfaceSDL2* frontSurface = nullptr;

class VideoSurfaceSDL2 : public VideoSurface
{
public:
    VideoSurfaceSDL2(int w, int h, GBC_Enum flags)
        : flags(flags)
        , windowSurface(nullptr)
        , texture(nullptr)
        , png_texture(nullptr)
    {
        surface = SDL_CreateRGBSurface(0, w, h, 8, 0, 0, 0, 0);
        SDL_SetSurfacePalette(surface, palette);

        if (flags & GBC_VISIBLE) {
            windowSurface = SDL_CreateRGBSurfaceWithFormat(0, w, h, SDL_BITSPERPIXEL(pixel_format), pixel_format);
            texture = SDL_CreateTexture(renderer, windowSurface->format->format, SDL_TEXTUREACCESS_STREAMING, w, h);
            frontSurface = this;
        }
    }

    virtual ~VideoSurfaceSDL2()
    {
        if (frontSurface == this) {
            frontSurface = nullptr;
        }

        if (surface != nullptr) {
            SDL_FreeSurface(surface);
        }

        if (windowSurface != nullptr) {
            SDL_FreeSurface(windowSurface);
        }
    }

    virtual void* GetData() const
    {
        return surface->pixels;
    }
    virtual int GetPitch() const
    {
        return surface->pitch;
    }
    virtual bool IsAllocated() const
    {
        return false;
    }

    virtual void AddAttachedSurface(VideoSurface* surface)
    {
    }

    virtual bool IsReadyToBlit()
    {
        return true;
    }

    virtual bool LockWait()
    {
        return (SDL_LockSurface(surface) == 0);
    }

    virtual bool Unlock()
    {
        SDL_UnlockSurface(surface);
        return true;
    }

    virtual void Blt(const Rect& destRect, VideoSurface* src, const Rect& srcRect, bool mask)
    {
        SDL_BlitSurface(((VideoSurfaceSDL2*)src)->surface, (SDL_Rect*)(&srcRect), surface, (SDL_Rect*)&destRect);
    }

    virtual void FillRect(const Rect& rect, unsigned char color)
    {
        SDL_Rect rectSDL = {rect.X, rect.Y, rect.Width + 1, rect.Height + 1};
        SDL_FillRect(surface, &rectSDL, color);
    }

    virtual bool Load_Png_Image(const Rect& dest_rect, const char* png_file_path)
    {
        if (windowSurface == nullptr) {
            return false;
        }

        Clear_Png_Image();

        const auto img_surface = IMG_Load(png_file_path);

        if (img_surface == nullptr) {
            CNC_LOG_ERROR("SDL2 IMG_Load error: {}", SDL_GetError());
            return false;
        }

        SDL_SetSurfaceBlendMode(img_surface, SDL_BLENDMODE_BLEND);
        png_texture = SDL_CreateTextureFromSurface(renderer, img_surface);
        SDL_FreeSurface(img_surface);

        if (png_texture == nullptr) {
            CNC_LOG_ERROR("SDL2 SDL_CreateTextureFromSurface error: {}", SDL_GetError());
            return false;
        }

        png_rect.x = dest_rect.X;
        png_rect.y = dest_rect.Y;
        png_rect.w = dest_rect.Width + 1;
        png_rect.h = dest_rect.Height + 1;

        return true;
    }

    virtual void Clear_Png_Image()
    {
        if (png_texture == nullptr) {
            return;
        }

        SDL_DestroyTexture(png_texture);
        png_texture = nullptr;
    }

    virtual bool Capture_Frame(const char* output_file_path)
    {
        if (windowSurface == nullptr) {
            return false;
        }

        if (IMG_SavePNG(windowSurface, output_file_path) != 0) {
            CNC_LOG_ERROR("SDL2 IMG_SavePNG error: {}", SDL_GetError());
            return false;
        }

        return true;
    }

    virtual bool Capture_Sub_Frame(const Rect& sub_area, const char* output_file_path)
    {
        if (windowSurface == nullptr) {
            return false;
        }

        const SDL_Rect sdl_sub_area = { .x = sub_area.X, .y = sub_area.Y, .w = sub_area.Width, .h = sub_area.Height };
        auto sub_surface = SDL_CreateRGBSurfaceWithFormat(
            0,
            sdl_sub_area.w,
            sdl_sub_area.h,
            SDL_BITSPERPIXEL(pixel_format),
            pixel_format
        );
        SDL_BlitSurface(windowSurface, &sdl_sub_area, sub_surface, nullptr);

        const auto save_result = IMG_SavePNG(sub_surface, output_file_path) == 0;

        SDL_FreeSurface(sub_surface);

        if (!save_result) {
            CNC_LOG_ERROR("SDL2 IMG_SavePNG error: {}", SDL_GetError());
        }

        return save_result;
    }

    void RenderSurface()
    {
        void* pixels;
        int pitch;

        SDL_BlitSurface(surface, NULL, windowSurface, NULL);

        if (Settings.Video.HardwareCursor) {
            /*
            ** Swap cursor before a frame is drawn. This reduces flickering when it's done only once per frame.
            */
            if (hwcursor.Pending) {
                SDL_SetCursor(hwcursor.Pending);

                if (hwcursor.Current) {
                    SDL_FreeCursor(hwcursor.Current);
                }

                hwcursor.Current = hwcursor.Pending;
                hwcursor.Pending = nullptr;
            }

            /*
            ** Update hardware cursor visibility.
            */
            SDL_ShowCursor(!Get_Mouse_State());
        } else if (!Get_Mouse_State() && hwcursor.Surface != nullptr) {
            /*
            ** Draw software emulated cursor.
            */
            int x, y;
            SDL_Rect dst;

            Get_Video_Mouse(x, y);

            dst.x = x - hwcursor.HotX;
            dst.y = y - hwcursor.HotY;
            dst.w = hwcursor.Surface->w;
            dst.h = hwcursor.Surface->h;

            SDL_BlitSurface(hwcursor.Surface, nullptr, windowSurface, &dst);
        }

        SDL_UpdateTexture(texture, NULL, windowSurface->pixels, windowSurface->pitch);
        SDL_RenderClear(renderer);

        if (CurrentResolutionMode == MODE_ORIGINAL_RES) {
            constexpr SDL_Rect src_rect = {0, 0, DefaultWidth, DefaultHeight};

            if (StretchOriginalResolution) {
                // 'stretch' the content to fill the screen
                SDL_RenderCopy(renderer, texture, &src_rect, &render_dst);
            } else {
                const SDL_Rect center_dst = {
                    .x = (render_dst.w / 2) - (DefaultWidth / 2),
                    .y = (render_dst.h / 2) - (DefaultHeight / 2),
                    .w = DefaultWidth,
                    .h = DefaultHeight
                };

                // render the content in the center of the screen
                SDL_RenderCopy(renderer, texture, &src_rect, &center_dst);
            }
        } else {
            SDL_RenderCopy(renderer, texture, nullptr, &render_dst);
        }

        Render_Png_Texture();

        SDL_RenderPresent(renderer);
    }

    int GetWidth()
    {
        return surface == nullptr ? 0 : surface->w;
    }

    int GetHeight()
    {
        return surface == nullptr ? 0 : surface->h;
    }

private:
    SDL_Surface* surface;
    SDL_Surface* windowSurface;
    SDL_Texture* texture;
    SDL_Texture* png_texture;
    SDL_Rect png_rect;
    GBC_Enum flags;

    void Render_Png_Texture() const
    {
        if (png_texture == nullptr) {
            return;
        }

        SDL_Rect draw_rect = png_rect;

        if (CurrentResolutionMode == MODE_ORIGINAL_RES) {
            if (StretchOriginalResolution) {
                const auto scale_x = static_cast<float>(render_dst.w) / DefaultWidth;
                const auto scale_y = static_cast<float>(render_dst.h) / DefaultHeight;

                draw_rect.x = static_cast<int>(static_cast<float>(png_rect.x) * scale_x);
                draw_rect.y = static_cast<int>(static_cast<float>(png_rect.y) * scale_y);
                draw_rect.w = static_cast<int>(static_cast<float>(png_rect.w) * scale_x);
                draw_rect.h = static_cast<int>(static_cast<float>(png_rect.h) * scale_y);
            } else {
                draw_rect.x = static_cast<int>(
                    static_cast<float>(png_rect.x)
                    + (static_cast<float>(render_dst.x) - DefaultWidth / 2.0f)
                );
                draw_rect.y = static_cast<int>(
                    static_cast<float>(png_rect.y)
                    + (static_cast<float>(render_dst.y) - DefaultHeight / 2.0f)
                );
            }
        } else {
            int game_w, game_h;
            const auto res_mode = Get_Current_Resolution_Mode();

            if (res_mode == MODE_HIGH_RES) {
                game_w = Settings.Video.Width;
                game_h = Settings.Video.Height;
            } else if (res_mode == MODE_DEFAULT) {
                game_w = DefaultWidth;
                game_h = DefaultHeight;
            } else {
                game_w = DefaultDosWidth;
                game_h = DefaultDosHeight;
            }

            if (StretchOriginalResolution) {
                const auto scale_x = static_cast<float>(render_dst.w) / static_cast<float>(game_w);
                const auto scale_y = static_cast<float>(render_dst.h) / static_cast<float>(game_h);

                draw_rect.x = static_cast<int>(static_cast<float>(png_rect.x) * scale_x);
                draw_rect.y = static_cast<int>(static_cast<float>(png_rect.y) * scale_y);
                draw_rect.w = static_cast<int>(static_cast<float>(png_rect.w) * scale_x);
                draw_rect.h = static_cast<int>(static_cast<float>(png_rect.h) * scale_y);
            } else {
                const auto offset_x = render_dst.x + (render_dst.w - game_w) / 2;
                const auto offset_y = render_dst.y + (render_dst.h - game_h) / 2;

                draw_rect.x = png_rect.x + offset_x;
                draw_rect.y = png_rect.y + offset_y;
            }
        }

        SDL_RenderCopy(renderer, png_texture, nullptr, &draw_rect);
    }
};

void Video_Render_Frame()
{
    if (frontSurface) {
        frontSurface->RenderSurface();
    }
}

/*
** Video
*/

Video::Video()
{
}

Video::~Video()
{
}

Video& Video::Shared()
{
    static Video video;
    return video;
}

VideoSurface* Video::CreateSurface(int w, int h, GBC_Enum flags)
{
    return new VideoSurfaceSDL2(w, h, flags);
}

/* Resolution mode API */

ResolutionMode Get_Current_Resolution_Mode()
{
    return CurrentResolutionMode;
}

void Set_Current_Resolution_Mode(const ResolutionMode resolution_mode, const bool enable_original_res_stretch)
{
    CurrentResolutionMode = resolution_mode;
    StretchOriginalResolution = enable_original_res_stretch;
}

void Set_Current_Resolution_Mode(const ResolutionMode resolution_mode)
{
    CurrentResolutionMode = resolution_mode;
}

std::optional<int> Try_Get_Resolution_Mode_Width()
{
    if (frontSurface == nullptr) {
        return std::nullopt;
    }

    if (Get_Current_Resolution_Mode() == MODE_ORIGINAL_RES) {
        return DefaultWidth;
    }

    return frontSurface->GetWidth();
}

std::optional<int> Try_Get_Resolution_Mode_Height()
{
    if (frontSurface == nullptr) {
        return std::nullopt;
    }

    if (Get_Current_Resolution_Mode() == MODE_ORIGINAL_RES) {
        return DefaultHeight;
    }

    return frontSurface->GetHeight();
}

void Enter_Original_Resolution_Mode()
{
    const auto resolution_mode = Get_Current_Resolution_Mode();

    if (resolution_mode == MODE_DOS || resolution_mode == MODE_DEFAULT) {
        // these modes never change
        return;
    }

    Set_Current_Resolution_Mode(MODE_ORIGINAL_RES);

    // center mouse when rendering content designed for original resolution
    if (StretchOriginalResolution) {
        Move_Video_Mouse_Absolute(DefaultWidth / 2, DefaultHeight / 2);
    } else if (frontSurface != nullptr) {
        Move_Video_Mouse_Absolute(frontSurface->GetWidth() / 2, frontSurface->GetHeight() / 2);
    }
}

void Leave_Original_Resolution_Mode()
{
    const auto resolution_mode = Get_Current_Resolution_Mode();

    if (resolution_mode == MODE_DOS || resolution_mode == MODE_DEFAULT) {
        // these modes never change
        return;
    }

    Set_Current_Resolution_Mode(MODE_HIGH_RES);

    if (frontSurface == nullptr) {
        CNC_LOG_WARN("Failed to center mouse due to uninitialised video surface");
        return;
    }

    // center mouse on screen
    Move_Video_Mouse_Absolute(frontSurface->GetWidth() / 2, frontSurface->GetHeight() / 2);
}
