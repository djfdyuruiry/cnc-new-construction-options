#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
class INIClass;

class SettingsClass
{
public:
    SettingsClass();

    void Load(INIClass& ini);
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
        int WindowWidth;
        int WindowHeight;
        bool Windowed;
        bool Boxing;
        std::string BoxingAspectRatio;
        int Display;
        int Width;
        int Height;
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
};

extern SettingsClass Settings;

#endif
