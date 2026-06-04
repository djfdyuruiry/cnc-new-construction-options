#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

#include "logger.h"

class INIClass;

class SettingsClass
{
public:
    SettingsClass();

    void Load(std::string ini_file_name, INIClass& ini);
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

private:
    static inline const auto& Logger = CncLogger::For(Settings);

    std::string IniFileName;
};

#endif

extern SettingsClass Settings;
