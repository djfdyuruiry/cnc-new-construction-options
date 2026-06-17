#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

#include "logger.h"
#include "rulesections.h"

class INIClass;

/**
 * Common settings for Tiberian Dawn and Red Alert.
 *
 * Game engines call ::Get_Sections to add their own engine
 * specific sections and settings.
 */
class SettingsClass
{
public:
    SettingsClass();

    void Load(std::string ini_file_name, INIClass& ini);
    void Update_Sections();
    void Save(INIClass& ini);

    struct
    {
        bool RawInput;
        int Sensitivity;
        bool ControllerEnabled;
        int ControllerPointerSpeed;
    } Mouse;

    struct
    {
        int Width;
        int Height;
        int StretchWidth;
        int StretchHeight;
        bool Windowed;
        bool Boxing;
        std::string BoxingAspectRatio;
        int Display;
        int FrameLimit;
        int InterpolationMode;
        bool HardwareCursor;
        bool DOSMode;
        int ButtonStyle;
        std::string Scaler;
        std::string VideoDriver;
        std::string RenderDriver;
        std::string PixelFormat;
    } Video;

    struct
    {
        bool MouseWheelScrolling;
    } Options;

    RuleSections& Get_Sections();

private:
    static inline const auto& Logger = CncLogger::For(Settings);

    std::string IniFileName;
    RuleSections Sections;
};

#endif

extern SettingsClass Settings;
