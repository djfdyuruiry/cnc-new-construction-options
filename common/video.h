#ifndef VIDEO_H
#define VIDEO_H

#include "rect.h"
#include <cstdint>
#include <optional>

enum GBC_Enum
{
    GBC_NONE = 0,
    GBC_VIDEOMEM = 1,
    GBC_VISIBLE = 2,
};

class VideoSurface;

class Video
{
public:
    Video();
    virtual ~Video();

    static Video& Shared();

    VideoSurface* CreateSurface(int w, int h, GBC_Enum flags);
};

class VideoSurface
{
public:
    virtual ~VideoSurface()
    {
    }

    virtual void* GetData() const = 0;
    virtual int GetPitch() const = 0;
    virtual bool IsAllocated() const = 0;

    virtual void AddAttachedSurface(VideoSurface* surface) = 0;
    virtual bool IsReadyToBlit() = 0;
    virtual bool LockWait() = 0;
    virtual bool Unlock() = 0;
    virtual void Blt(const Rect& destRect, VideoSurface* src, const Rect& srcRect, bool mask) = 0;
    virtual void FillRect(const Rect& rect, unsigned char color) = 0;
};

class SurfaceMonitorClass
{
public:
    SurfaceMonitorClass();
    virtual ~SurfaceMonitorClass()
    {
    }

    virtual void Restore_Surfaces() = 0;
    virtual void Set_Surface_Focus(bool in_focus) = 0;
    virtual void Release() = 0;

    bool SurfacesRestored;
};

extern SurfaceMonitorClass& AllSurfaces; // List of all surfaces

bool Set_Video_Mode(int& w, int& h, int bits_per_pixel);
void Get_Video_Scale(float& x, float& y);
void Set_Video_Cursor_Clip(bool clipped);
void Move_Video_Mouse(float xrel, float yrel);
void Move_Video_Mouse_Absolute(int x, int y);
void Get_Video_Mouse(int& x, int& y);
void Toggle_Video_Fullscreen();
void Reset_Video_Mode();
unsigned Get_Free_Video_Memory();
void Wait_Blit();

/*
** Set desired cursor image in game palette.
*/
void Set_Video_Cursor(void* cursor, int w, int h, int hotx, int hoty);

/*
 *  Flags returned by Get_Video_Hardware_Capabilities
 */
/* Hardware blits supported? */
#define VIDEO_BLITTER 1

/* Hardware blits asyncronous? */
#define VIDEO_BLITTER_ASYNC 2

/* Can palette changes be synced to vertical refresh? */
#define VIDEO_SYNC_PALETTE 4

/* Is the video cards memory bank switched? */
#define VIDEO_BANK_SWITCHED 8

/* Can the blitter do filled rectangles? */
#define VIDEO_COLOR_FILL 16

/* Is there no hardware assistance avaailable at all? */
#define VIDEO_NO_HARDWARE_ASSIST 32

unsigned Get_Video_Hardware_Capabilities();

void Wait_Vert_Blank();
void Set_DD_Palette(void* palette);

/* Resolution mode control */

enum ResolutionMode
{
    // original resolution (640x400)
    MODE_DEFAULT,
    // original DOS resolution (300x200)
    MODE_DOS,
    // custom resolution
    MODE_HIGH_RES,
    // zoom into default resolution
    MODE_ZOOM
};

ResolutionMode Get_Current_Resolution_Mode();

void Set_Current_Resolution_Mode(ResolutionMode resolution_mode);

/**
 * Lookup the width for the current resolution mode. This can be a subset of the current
 * internal resolution (640x400) or the full resolution. Useful to ensure a relative point
 * on the screen is in the correct position of the currently viewed screen portion. Used
 * for dialogs that can be shown on 640x400 screen and in scenario gameplay (save/load etc.).
 *
 * Supported video backends: sdl2
 *
 * @return Width or std::nullopt if video system is uninitialised or using an unsupported video backed.
 */
std::optional<int> Try_Get_Resolution_Mode_Width();

/**
 * Lookup the height for the current resolution mode. This can be a subset of the current
 * internal resolution (640x400) or the full resolution. Useful to ensure a relative point
 * on the screen is in the correct position of the currently viewed screen portion. Used
 * for dialogs that can be shown on 640x400 screen and in scenario gameplay (save/load etc.).
 *
 * Supported video backends: sdl2
 *
 * @return Width or std::nullopt if video system is uninitialised or using an unsupported video backed.
 */
std::optional<int> Try_Get_Resolution_Mode_Height();

/**
 * Enter the standard resolution mode for the game engine, if current resolution mode supports it.
 *
 * Generally this should be called after clearing the screen to prevent zoom in artifacts.
 *
 * This mode is currently used for menus, videos, CPS animations and score screens.
 *
 * Supported video backends: sdl2
*/
void Enter_Zoomed_Resolution_Mode();

/**
 * Enter the dynamic high resolution mode for the game engine, if current resolution mode supports it.
 *
 * Call this to reset calls to Enter_Standard_Resolution_Mode().
 *
 * This mode is currently only used when playing a scenario.
 *
 * Supported video backends: sdl2
 */
void Leave_Zoomed_Resolution_Mode();

void Toggle_Imgui();

#endif // VIDEO_H
