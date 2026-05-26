#include "wwstd.h"
#include "settings.h"
#include "ini.h"
#include "miscasm.h"

SettingsClass Settings;

SettingsClass::SettingsClass()
{
    /*
    ** Mouse settings
    */
    Mouse.RawInput = true;
    Mouse.Sensitivity = 100;
    Mouse.ControllerEnabled = false;
    Mouse.ControllerPointerSpeed = 10;
    Options.MouseWheelScrolling = true;

    /*
    ** Video settings
    */
    // TODO: Could offer presets through the launcher or a ini setting (retro, modern etc.)
    Video.Width = 640;
    Video.Height = 400;
    Video.StretchWidth = 0;
    Video.StretchHeight = 0;
    Video.Windowed = false;
    Video.Display = 1;
    Video.Boxing = true;
    Video.BoxingAspectRatio = "16:10";
    Video.FrameLimit = 120;
    Video.InterpolationMode = 2;
    Video.HardwareCursor = false;
    Video.DOSMode = false;
    Video.Scaler = "linear";
    Video.VideoDriver = "default";
    Video.RenderDriver = "default";
    Video.PixelFormat = "default";
}

void SettingsClass::Load(INIClass& ini)
{
    char buf[128];

    /*
    ** Mouse settings
    */
    Mouse.RawInput = ini.Get_Bool("Mouse", "RawInput", Mouse.RawInput);
    Mouse.Sensitivity = ini.Get_Int("Mouse", "Sensitivity", Mouse.Sensitivity);
    Mouse.ControllerEnabled = ini.Get_Bool("Mouse", "ControllerEnabled", Mouse.ControllerEnabled);
    Mouse.ControllerPointerSpeed = ini.Get_Int("Mouse", "ControllerPointerSpeed", Mouse.ControllerPointerSpeed);
    /*
    ** Compatibility with CNCNet configuration for this feature
    */
    Options.MouseWheelScrolling = ini.Get_Bool("Options", "MouseWheelScrolling", Options.MouseWheelScrolling);
    Options.MouseWheelScrolling = ini.Get_Bool("Mouse", "MouseWheelScrolling", Options.MouseWheelScrolling);

    /*
    ** Video settings
    */
    Video.Width = ini.Get_Int("Video", "Width", Video.Width);
    Video.Height = ini.Get_Int("Video", "Height", Video.Height);
    Video.StretchWidth = ini.Get_Int("Video", "StretchWidth", Video.StretchWidth);
    Video.StretchHeight = ini.Get_Int("Video", "StretchHeight", Video.StretchHeight);
    Video.Windowed = ini.Get_Bool("Video", "Windowed", Video.Windowed);
    Video.Boxing = ini.Get_Bool("Video", "Boxing", Video.Boxing);
    Video.BoxingAspectRatio = ini.Get_String("Video", "BoxingAspectRatio", Video.BoxingAspectRatio);
    Video.Display = ini.Get_Int("Video", "Display", Video.Display);
    Video.FrameLimit = ini.Get_Int("Video", "FrameLimit", Video.FrameLimit);
    Video.HardwareCursor = ini.Get_Bool("Video", "HardwareCursor", Video.HardwareCursor);
    Video.DOSMode = ini.Get_Bool("Video", "DOSMode", Video.DOSMode);
    Video.Scaler = ini.Get_String("Video", "Scaler", Video.Scaler);

#if !defined(_WIN32) && !defined(__APPLE__)
    Video.VideoDriver = ini.Get_String("Video", "VideoDriver", Video.VideoDriver);
#endif

    Video.RenderDriver = ini.Get_String("Video", "RenderDriver", Video.RenderDriver);
    Video.PixelFormat = ini.Get_String("Video", "PixelFormat", Video.PixelFormat);

    /*
    ** VQA and WSA interpolation mode 0 = scanlines, 1 = vertical doubling, 2 = linear
    */
    Video.InterpolationMode = Bound(ini.Get_Int("Video", "InterpolationMode", Video.InterpolationMode), 0, 2);

    /*
    ** Boxing and raw input require software cursor.
    */
    if (Video.Boxing || Mouse.RawInput || Mouse.ControllerEnabled) {
        Video.HardwareCursor = false;
    }

    ini.Get_String("Video", "ButtonStyle", "Default", buf, sizeof(buf));
    if (!stricmp(buf, "Gold")) {
        Video.ButtonStyle = 1;
    } else if (!stricmp(buf, "Classic") || !stricmp(buf, "DOS")) {
        Video.ButtonStyle = 0;
    } else {
        Video.ButtonStyle = -1;
    }
}

/**
 * Load settings, but use a high resolution as the
 * default if none is defined in the INI.
 */
void SettingsClass::Load_Hi_Res(INIClass& ini)
{
    Video.Width = 800;
    Video.Height = 600;

    Load(ini);
}

void SettingsClass::Save(INIClass& ini)
{
    /*
    ** Mouse settings
    */
    ini.Put_Bool("Mouse", "RawInput", Mouse.RawInput);
    ini.Put_Int("Mouse", "Sensitivity", Mouse.Sensitivity);
    ini.Put_Bool("Mouse", "ControllerEnabled", Mouse.ControllerEnabled);
    ini.Put_Int("Mouse", "ControllerPointerSpeed", Mouse.ControllerPointerSpeed);
    ini.Put_Bool("Mouse", "MouseWheelScrolling", Options.MouseWheelScrolling);

    /*
    ** Video settings
    */
    ini.Put_Int("Video", "Width", Video.Width);
    ini.Put_Int("Video", "Height", Video.Height);
    ini.Put_Int("Video", "StretchWidth", Video.StretchWidth);
    ini.Put_Int("Video", "StretchHeight", Video.StretchHeight);
    ini.Put_Bool("Video", "Windowed", Video.Windowed);
    ini.Put_Bool("Video", "Boxing", Video.Boxing);
    ini.Put_String("Video", "BoxingAspectRatio", Video.BoxingAspectRatio, "4:3, 16:9 etc.");
    ini.Put_Int("Video", "Display", Video.Display);
    ini.Put_Int("Video", "FrameLimit", Video.FrameLimit);
    ini.Put_Bool("Video", "HardwareCursor", Video.HardwareCursor);
    ini.Put_Bool("Video", "DOSMode", Video.DOSMode, "before you enable this, install the game data files from the DOS version");
    ini.Put_String("Video", "Scaler", Video.Scaler, "nearest (sharp), linear (smooth)");

#if !defined(_WIN32) && !defined(__APPLE__)
    ini.Put_String("Video", "VideoDriver", Video.VideoDriver, "default, x11, wayland, directfb, kmsdrm");
#endif

#ifdef _WIN32
    ini.Put_String("Video", "RenderDriver", Video.RenderDriver, "default, direct3d, direct3d11, direct3d12, opengl, software");
#elifdef __APPLE__
    ini.Put_String("Video", "RenderDriver", Video.RenderDriver, "default, metal, software");
#else
    ini.Put_String("Video", "RenderDriver", Video.RenderDriver, "default, opengl, opengles2, opengles, software");
#endif

    ini.Put_String("Video", "PixelFormat", Video.PixelFormat);

    /*
    ** VQA and WSA interpolation mode 0 = scanlines, 1 = vertical doubling, 2 = linear
    */
    ini.Put_Int("Video", "InterpolationMode", Video.InterpolationMode);

    ini.Put_String(
        "Video",
        "ButtonStyle",
        Video.ButtonStyle == -1 ? "Default" : (Video.ButtonStyle == 1 ? "Gold" : "Classic"),
        "Default, Gold, Classic, DOS"
    );
}
