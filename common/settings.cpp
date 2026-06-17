#include "wwstd.h"
#include "settings.h"
#include "ini.h"
#include "logger.h"
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
    Video.Width = 800;
    Video.Height = 600;
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

void SettingsClass::Load(std::string ini_file_name, INIClass& ini)
{
    IniFileName = std::move(ini_file_name);

    CNC_LOG_INFO("Loading common settings from INI file: {}", IniFileName);

    /*
    ** Mouse settings
    */
    Sections["Mouse"].With<IniRuleContext>(ini, [&](auto& c) {
         c.Load("RawInput").With_Binding(Mouse.RawInput)
          .Load("Sensitivity").With_Binding(Mouse.Sensitivity)
          .Load("ControllerEnabled").With_Binding(Mouse.ControllerEnabled)
          .Load("ControllerPointerSpeed").With_Binding(Mouse.ControllerPointerSpeed)
          .Load("MouseWheelScrolling").With_Binding(Options.MouseWheelScrolling);
    });

    /*
    ** Compatibility with CNCNet configuration for this feature
    */
    Sections["Options"].With<IniRuleContext>(ini, [&](auto& c) {
        c.Load("MouseWheelScrolling").With_Binding(Options.MouseWheelScrolling);
    });

    /*
    ** Video settings
    */
    Sections["Video"].With<IniRuleContext>(ini, [&](auto& c) {
        c.Load("VideoWidth").With_Binding(Video.Width)
         .Load("VideoHeight").With_Binding(Video.Height)
         .Load("StretchWidth").With_Binding(Video.StretchWidth)
         .Load("StretchHeight").With_Binding(Video.StretchHeight)
         .Load("Windowed").With_Binding(Video.Windowed)
         .Load("Boxing").With_Binding(Video.Boxing)
         .Load("BoxingAspectRatio").With_Comment("4:3, 16:9 etc.").With_Binding(Video.BoxingAspectRatio)
         .Load("Display").With_Binding(Video.Display)
         .Load("FrameLimit").With_Binding(Video.FrameLimit)
         .Load("HardwareCursor").With_Binding(Video.HardwareCursor)
         .Load("DOSMode")
            .With_Comment("before you enable this, install the game data files from the DOS version")
            .With_Binding(Video.DOSMode)
         .Load("Scaler").With_Comment("nearest (sharp), linear (smooth)").With_Binding(Video.Scaler)

#if !defined(_WIN32) && !defined(__APPLE__)
         .Load("VideoDriver").With_Comment("default, x11, wayland, directfb, kmsdrm").With_Binding(Video.VideoDriver)
#endif

#ifdef _WIN32
         .Load("RenderDriver")
            .With_Comment("default, direct3d, direct3d11, direct3d12, opengl, software")
            .With_Binding(Video.RenderDriver)
#elifdef __APPLE__
         .Load("RenderDriver").With_Comment("default, metal, software").With_Binding(Video.RenderDriver)
#else
         .Load("RenderDriver")
            .With_Comment("default, opengl, opengles2, opengles, software")
            .With_Binding(Video.RenderDriver)
#endif

         .Load("PixelFormat").With_Binding(Video.PixelFormat)
         .Load("InterpolationMode").With_Binding(Video.InterpolationMode)
         .Load("ButtonStyle").With_Default("Default");
    });

    /*
    ** VQA and WSA interpolation mode 0 = scanlines, 1 = vertical doubling, 2 = linear
    */
    Video.InterpolationMode = Bound(Video.InterpolationMode, 0, 2);

    /*
    ** Boxing and raw input require software cursor.
    */
    if (Video.Boxing || Mouse.RawInput || Mouse.ControllerEnabled) {
        Video.HardwareCursor = false;
    }

    const auto button_style = Sections["Video"].Get<std::string>("ButtonStyle");

    if (button_style == "Gold") {
        Video.ButtonStyle = 1;
    } else if (button_style == "Classic" || button_style == "DOS") {
        Video.ButtonStyle = 0;
    } else {
        Video.ButtonStyle = -1;
    }
}

void SettingsClass::Update_Sections()
{
    CNC_LOGGER_DEBUG("Updating common settings from SettingsClass fields");

    /*
    ** Mouse settings
    */
    Sections["Mouse"]
        .Set("RawInput", Mouse.RawInput)
        .Set("Sensitivity", Mouse.Sensitivity)
        .Set("ControllerEnabled", Mouse.ControllerEnabled)
        .Set("ControllerPointerSpeed", Mouse.ControllerPointerSpeed)
        .Set("MouseWheelScrolling", Options.MouseWheelScrolling);

    /*
    ** Compatibility with CNCNet configuration for this feature
    */
    Sections["Options"].Set("MouseWheelScrolling", Options.MouseWheelScrolling);

    /*
    ** Video settings
    */
    Sections["Video"]
        .Set("VideoWidth", Video.Width)
        .Set("VideoHeight", Video.Height)
        .Set("StretchWidth", Video.StretchWidth)
        .Set("StretchHeight", Video.StretchHeight)
        .Set("Windowed", Video.Windowed)
        .Set("Boxing", Video.Boxing)
        .Set("BoxingAspectRatio", Video.BoxingAspectRatio)
        .Set("Display", Video.Display)
        .Set("FrameLimit", Video.FrameLimit)
        .Set("HardwareCursor", Video.HardwareCursor)
        .Set("DOSMode", Video.DOSMode)
        .Set("Scaler", Video.Scaler)
#if !defined(_WIN32) && !defined(__APPLE__)
        .Set("VideoDriver", Video.VideoDriver)
#endif
        .Set("RenderDriver", Video.RenderDriver)
        .Set("PixelFormat", Video.PixelFormat)
        .Set("InterpolationMode", Video.InterpolationMode)
        .Set(
            "ButtonStyle",
            Video.ButtonStyle == -1 ? "Default" : (Video.ButtonStyle == 1 ? "Gold" : "Classic")
        );
}

void SettingsClass::Save(INIClass& ini)
{
    CNC_LOGGER_INFO("Saving common settings to INI file: {}", IniFileName);

    Update_Sections();

    Sections.Save_All_To_Ini(ini);
}

RuleSections& SettingsClass::Get_Sections()
{
    return Sections;
}
