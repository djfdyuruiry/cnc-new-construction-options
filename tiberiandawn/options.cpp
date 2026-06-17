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

/* $Header:   F:\projects\c&c\vcs\code\options.cpv   2.17   16 Oct 1995 16:51:28   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  W E S T W O O D   S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : OPTIONS.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : June 8, 1994                                                 *
 *                                                                                             *
 *                  Last Update : June 30, 1995 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   OptionsClass::Adjust_Palette -- Adjusts the palette according to the settings specified.  *
 *   OptionsClass::Get_Brightness -- Fetches the current brightness setting.                   *
 *   OptionsClass::Get_Color -- Fetches the current color setting.                             *
 *   OptionsClass::Get_Contrast -- Gets the current contrast setting.                          *
 *   OptionsClass::Get_Game_Speed -- Fetches the current game speed setting.                   *
 *   OptionsClass::Get_Scroll_Rate -- Fetches the current scroll rate setting.                 *
 *   OptionsClass::Get_Tint -- Fetches the current tint setting.                               *
 *   OptionsClass::Load_Settings -- reads options settings from the INI file                   *
 *   OptionsClass::Normalize_Delay -- Normalizes delay factor to keep rate constant.           *
 *   OptionsClass::One_Time -- This performs any one time initialization for the options class.*
 *   OptionsClass::OptionsClass -- The default constructor for the options class.              *
 *   OptionsClass::Process -- Handles all the options graphic interface.                       *
 *   OptionsClass::Save_Settings -- writes options settings to the INI file                    *
 *   OptionsClass::Set -- Sets options based on current settings                               *
 *   OptionsClass::Set_Brightness -- Sets the brightness level to that specified.              *
 *   OptionsClass::Set_Color -- Sets the color to the value specified.                         *
 *   OptionsClass::Set_Contrast -- Sets the contrast to the value specified.                   *
 *   OptionsClass::Set_Game_Speed -- Sets the game speed as specified.                         *
 *   OptionsClass::Set_Repeat -- Controls the score repeat option.                             *
 *   OptionsClass::Set_Score_Volume -- Sets the global score volume to that specified.         *
 *   OptionsClass::Set_Scroll_Rate -- Sets the scroll rate as specified.                       *
 *   OptionsClass::Set_Shuffle -- Controls the play shuffle setting.                           *
 *   OptionsClass::Set_Sound_Volume -- Sets the sound effects volume level.                    *
 *   OptionsClass::Set_Tint -- Sets the tint setting.                                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "function.h"
#include "options.h"
#include "common/ini.h"

#ifdef SDL_BUILD
char const* const OptionsClass::HotkeyName = "SDLHotkeys";
#else
char const* const OptionsClass::HotkeyName = "WinHotkeys";
#endif

/***********************************************************************************************
 * OptionsClass::OptionsClass -- The default constructor for the options class.                *
 *                                                                                             *
 *    This is the constructor for the options class. It handles setting up all the globals     *
 *    necessary for the options. This includes setting them to their default state.            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/21/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
OptionsClass::OptionsClass(void)
    : KeyForceMove1(KN_LALT)
    , KeyForceMove2(KN_RALT)
    , KeyForceAttack1(KN_LCTRL)
    , KeyForceAttack2(KN_RCTRL)
    , KeySelect1(KN_LSHIFT)
    , KeySelect2(KN_RSHIFT)
    , KeyScatter(KN_X)
    , KeyStop(KN_S)
    , KeyGuard(KN_G)
    , KeyNext(KN_N)
    , KeyPrevious(KN_B)
    , KeyFormation(KN_F)
    , KeyHome1(KN_HOME)
    , KeyHome2(KN_E_HOME)
    , KeyBase(KN_H)
    , KeyResign(KN_R)
    , KeyAlliance(KN_A)
    , KeyBookmark1(KN_F9)
    , KeyBookmark2(KN_F10)
    , KeyBookmark3(KN_F11)
    , KeyBookmark4(KN_F12)
    , KeySelectView(KN_E)
    , KeyRepair(KN_T)
    , KeyRepairOn(KN_NONE)
    , KeyRepairOff(KN_NONE)
    , KeySell(KN_Y)
    , KeySellOn(KN_NONE)
    , KeySellOff(KN_NONE)
    , KeyMap(KN_U)
    , KeySidebarUp(KN_UP)
    , KeySidebarDown(KN_DOWN)
    , KeyOption1(KN_ESC)
    , KeyOption2(KN_SPACE)
    , KeyScrollLeft(KN_NONE)
    , KeyScrollRight(KN_NONE)
    , KeyScrollUp(KN_NONE)
    , KeyScrollDown(KN_NONE)
    , KeyQueueMove1(KN_Q)
    , KeyQueueMove2(KN_Q)
    , KeyTeam1(KN_1)
    , KeyTeam2(KN_2)
    , KeyTeam3(KN_3)
    , KeyTeam4(KN_4)
    , KeyTeam5(KN_5)
    , KeyTeam6(KN_6)
    , KeyTeam7(KN_7)
    , KeyTeam8(KN_8)
    , KeyTeam9(KN_9)
    , KeyTeam10(KN_0)
    , KeyDeploy(KN_D)
    , KeySelectAllOfType(KN_T)
{
    GameSpeed = TIMER_SECOND / TICKS_PER_SECOND;
    ScrollRate = TIMER_SECOND / TICKS_PER_SECOND;
    Volume = 0xE0;
    ScoreVolume = 0x90;
    Contrast = 0x80;
    Color = 0x80;
    Contrast = 0x80;
    Tint = 0x80;
    Brightness = 0x80;
    AutoScroll = true;
#if (GERMAN | FRENCH)
    IsDeathAnnounce = true;
#else
    IsDeathAnnounce = false;
#endif
    IsScoreRepeat = false;
    IsScoreShuffle = false;
    IsFreeScroll = false;
    SkipExpansionCdCheck = true;
}

/***********************************************************************************************
 * OptionsClass::One_Time -- This performs any one time initialization for the options class.  *
 *                                                                                             *
 *    This routine should be called only once and it will perform any inializations for the    *
 *    options class that is needed. This may include things like file loading and memory       *
 *    allocation.                                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Only call this routine once.                                                    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/21/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void OptionsClass::One_Time(SettingsClass& settings)
{
    Set_Score_Vol(ScoreVolume);

    CommonSettings = &settings;
}

/***********************************************************************************************
 * OptionsClass::Process -- Handles all the options graphic interface.                         *
 *                                                                                             *
 *    This routine is the main control for the visual representation of the options            *
 *    screen. It handles the visual overlay and the player input.                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/21/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void OptionsClass::Process(void)
{
}

/***********************************************************************************************
 * OptionsClass::Set_Shuffle -- Controls the play shuffle setting.                             *
 *                                                                                             *
 *    This routine will control the score shuffle flag. The setting to use is provided as      *
 *    a parameter. When shuffling is on, the score play order is scrambled.                    *
 *                                                                                             *
 * INPUT:   on -- Should the shuffle option be activated?                                      *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void OptionsClass::Set_Shuffle(int on)
{
    IsScoreShuffle = on;
}

/***********************************************************************************************
 * OptionsClass::Set_Repeat -- Controls the score repeat option.                               *
 *                                                                                             *
 *    This routine is used to control whether scores repeat or not. The setting to use for     *
 *    the repeat flag is provided as a parameter.                                              *
 *                                                                                             *
 * INPUT:   on -- Should the scores repeat?                                                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void OptionsClass::Set_Repeat(int on)
{
    IsScoreRepeat = on;
}

/***********************************************************************************************
 * OptionsClass::Set_Score_Volume -- Sets the global score volume to that specified.           *
 *                                                                                             *
 *    This routine will set the global score volume to the value specified. The value ranges   *
 *    from zero to 255.                                                                        *
 *                                                                                             *
 * INPUT:   volume   -- The new volume setting to use for scores.                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void OptionsClass::Set_Score_Volume(int volume)
{
    volume = Bound(volume, 0, 255);
    ScoreVolume = volume;
    Set_Score_Vol(ScoreVolume);
}

/***********************************************************************************************
 * OptionsClass::Set_Sound_Volume -- Sets the sound effects volume level.                      *
 *                                                                                             *
 *    This routine will set the sound effect volume level as indicated. It can generate a      *
 *    sound effect for feedback purposes if desired. The volume setting can range from zero    *
 *    to 255. The value of 255 is the loudest.                                                 *
 *                                                                                             *
 * INPUT:   volume   -- The volume setting to use for the new value. 0 to 255.                 *
 *                                                                                             *
 *          feedback -- Should a feedback sound effect be generated?                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void OptionsClass::Set_Sound_Volume(int volume, int feedback)
{
    volume = Bound(volume, 0, 255);
    Volume = volume;
    if (feedback) {
        Sound_Effect(VOC_BLEEPY3);
    }
}

/***********************************************************************************************
 * OptionsClass::Set_Brightness -- Sets the brightness level to that specified.                *
 *                                                                                             *
 *    This routine will set the current brightness level to the value specified. This value    *
 *    can range from zero to 255, with 128 being the normal (default) brightness level.        *
 *                                                                                             *
 * INPUT:   brightness  -- The brightness level to set as current.                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void OptionsClass::Set_Brightness(int brightness)
{
    Brightness = 0x40 + Fixed_To_Cardinal(0x80, brightness);
    Adjust_Palette(OriginalPalette, GamePalette, Brightness, Color, Tint, Contrast);
    if (InMainLoop) {
        Set_Palette(GamePalette);
    }
}

/***********************************************************************************************
 * OptionsClass::Get_Brightness -- Fetches the current brightness setting.                     *
 *                                                                                             *
 *    This routine will fetch the current setting for the brightness level. The value ranges   *
 *    from zero to 255, with 128 being the normal (default) value.                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the current brightness setting.                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int OptionsClass::Get_Brightness(void) const
{
    return (Cardinal_To_Fixed(0x80, Brightness - 0x40));
}

/***********************************************************************************************
 * OptionsClass::Set_Color -- Sets the color to the value specified.                           *
 *                                                                                             *
 *    This routine will set the color value to that specified. The value specified can range   *
 *    from zero to 255. The value of 128 is the normal default color setting.                  *
 *                                                                                             *
 * INPUT:   color -- The new color value to set as current.                                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void OptionsClass::Set_Color(int color)
{
    Color = color;
    Adjust_Palette(OriginalPalette, GamePalette, Brightness, Color, Tint, Contrast);
    if (InMainLoop) {
        Set_Palette(GamePalette);
    }
}

/***********************************************************************************************
 * OptionsClass::Get_Color -- Fetches the current color setting.                               *
 *                                                                                             *
 *    This routine will fetch the current color setting. This value ranges from zero to        *
 *    255.                                                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the current color setting. The value of 128 is the normal (default)   *
 *          color setting.                                                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int OptionsClass::Get_Color(void) const
{
    return (Color);
}

/***********************************************************************************************
 * OptionsClass::Set_Contrast -- Sets the contrast to the value specified.                     *
 *                                                                                             *
 *    This routine will set the constrast to the setting specified. This setting ranges from   *
 *    zero to 255. The value o 128 is the normal default value.                                *
 *                                                                                             *
 * INPUT:   contrast -- The constrast setting to make as the current setting.                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void OptionsClass::Set_Contrast(int contrast)
{
    Contrast = 0x40 + Fixed_To_Cardinal(0x80, contrast);
    Adjust_Palette(OriginalPalette, GamePalette, Brightness, Color, Tint, Contrast);
    if (InMainLoop) {
        Set_Palette(GamePalette);
    }
}

/***********************************************************************************************
 * OptionsClass::Get_Contrast -- Gets the current contrast setting.                            *
 *                                                                                             *
 *    This routine will get the current contrast setting. The value returned is in the range   *
 *    of zero to 255.                                                                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the current contrast setting. A setting of 128 is the normal default value.*
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int OptionsClass::Get_Contrast(void) const
{
    return (Cardinal_To_Fixed(0x80, Contrast - 0x40));
}

/***********************************************************************************************
 * OptionsClass::Set_Tint -- Sets the tint setting.                                            *
 *                                                                                             *
 *    This routine will change the current tint setting according to the value specified.      *
 *                                                                                             *
 * INPUT:   tint  -- The desired tint setting. This value ranges from zero to 255.             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The value of 128 is the default (normal) tint setting.                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void OptionsClass::Set_Tint(int tint)
{
    Tint = tint;
    Adjust_Palette(OriginalPalette, GamePalette, Brightness, Color, Tint, Contrast);
    if (InMainLoop) {
        Set_Palette(GamePalette);
    }
}

/***********************************************************************************************
 * OptionsClass::Get_Tint -- Fetches the current tint setting.                                 *
 *                                                                                             *
 *    This fetches the current tint setting. The value is returned as a number between         *
 *    zero and 255. This has been adjusted for the valid range allowed.                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the current tint setting. Normal tint setting is 128.                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int OptionsClass::Get_Tint(void) const
{
    return (Tint);
}

/***********************************************************************************************
 * OptionsClass::Adjust_Palette -- Adjusts the palette according to the settings specified.    *
 *                                                                                             *
 *    This routine is used to adjust the palette according to the settings provided. It is     *
 *    used by the options class to monkey with the palette.                                    *
 *                                                                                             *
 * INPUT:   oldpal      -- Pointer to the original (unmodified) palette.                       *
 *                                                                                             *
 *          newpal      -- The new palette to create according to the settings provided.       *
 *                                                                                             *
 *          brightness  -- The brightness level (0..255).                                      *
 *                                                                                             *
 *          color       -- The color level (0..255).                                           *
 *                                                                                             *
 *          tint        -- The tint (hue) level (0..255).                                      *
 *                                                                                             *
 *          contrast    -- The contrast level (0..255).                                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/21/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void OptionsClass::Adjust_Palette(void* oldpal,
                                  void* newpal,
                                  unsigned char brightness,
                                  unsigned char color,
                                  unsigned char tint,
                                  unsigned char contrast) const
{
#ifndef REMASTER_BUILD
    int index;
    unsigned h, s, v;
    unsigned r, g, b;

    if (!oldpal || !newpal)
        return;

    /*
    **	Adjust for palette.
    */
    for (index = 0; index < 256; index++) {
        if (/*index == LTGREEN ||*/ index == 255) {
            memcpy(&((char*)newpal)[index * 3], &((char*)oldpal)[index * 3], 3);
        } else {
            r = ((char*)oldpal)[(index * 3) + 0];
            g = ((char*)oldpal)[(index * 3) + 1];
            b = ((char*)oldpal)[(index * 3) + 2];
            Convert_RGB_To_HSV(r, g, b, &h, &s, &v);

            /*
            **	Adjust contrast by moving the value toward the center according to the
            **	percentage indicated.
            */
            int temp;

            temp = (v * brightness) / 0x80; // Brightness
            temp = Bound(temp, 0, 0xFF);
            v = temp;
            temp = (((((int)v) - 0x80) * contrast) / 0x80) + 0x80; // Contrast
            temp = Bound(temp, 0, 0xFF);
            v = temp;
            temp = (s * color) / 0x80; // Color
            temp = Bound(temp, 0, 0xFF);
            s = temp;
            temp = (h * tint) / 0x80; // Tint
            temp = Bound(temp, 0, 0xFF);
            h = temp;
            Convert_HSV_To_RGB(h, s, v, &r, &g, &b);
            ((char*)newpal)[(index * 3) + 0] = r;
            ((char*)newpal)[(index * 3) + 1] = g;
            ((char*)newpal)[(index * 3) + 2] = b;
        }
    }
#endif
}

// helper macros for loading settings using RuleSections API

#define Load_Key_Binding(FIELD) template Load_With_Converter_Callback<KeyNumType, TdTypeConverter>(\
    #FIELD, \
    FIELD, \
    [&](const auto v) { \
        FIELD = static_cast<KeyNumType>(v & ~WWKEY_VK_BIT); \
    })

#define Load_Obfusticated_Setting(SETTING, LOOKUP, FIELD, VALUE)  template Load_With_Callback<std::string>(\
    SETTING, \
    "", \
    [&](auto v) { \
        if (Obfuscate(v.c_str()) == LOOKUP) { \
            FIELD = VALUE; \
        } \
    } \
)
#define Load_Special_Setting(SETTING, LOOKUP, FIELD, VALUE) Load_Obfusticated_Setting(SETTING, LOOKUP, Special.FIELD, VALUE)

/***********************************************************************************************
 * OptionsClass::Load_Settings -- reads options settings from the INI file                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *=============================================================================================*/
void OptionsClass::Load_Settings()
{
    /*
    **	Create filename and read the file.
    */
    CCFileClass file(CONFIG_FILE_NAME);
    INIClass ini;
    ini.Load(file);

    /*
    **	Read in the Options values
    */
    auto& options_section = Get_Options_Section();

    options_section.With<IniRuleContext>(ini, [&](auto& c) {
        c.Load_Int_Var_With_Default(GameSpeed, 4)
         .Load_Int_Var_With_Default(ScrollRate, 4)
         .Load_Int_Var_With_Default(Brightness, 0x80)
         .Load_Int_Var_With_Default(Volume, 0xA0)
         .Load_Int_Var_With_Default(ScoreVolume, 0xFF)
         .Load_Int_Var_With_Default(Contrast, 0x80)
         .Load_Int_Var_With_Default(Color, 0x80)
         .Load_Int_Var_With_Default(Tint, 0x80)
         .Load_Bool_Var_With_Default(AutoScroll, true)
         .Load_Bool_Var_With_Default(IsScoreRepeat, false)
         .Load_Bool_Var_With_Default(IsScoreShuffle, false)
         .template Load_With_Callback<bool>("DeathAnnounce", false, [&](const bool v) {
             IsDeathAnnounce = v;
         })
         .template Load_With_Callback<bool>("FreeScrolling", false, [&](const bool v) {
             IsFreeScroll = v;
         })
         .Load_Bool_Var_With_Default(SkipExpansionCdCheck, true)
         .Load_Bool_Var_With_Default(SlowPalette, true);

        // key bindings
        c.Load_Key_Binding(KeyForceMove1)
         .Load_Key_Binding(KeyForceMove2)
         .Load_Key_Binding(KeyForceAttack1)
         .Load_Key_Binding(KeyForceAttack2)
         .Load_Key_Binding(KeySelect1)
         .Load_Key_Binding(KeySelect2)
         .Load_Key_Binding(KeyScatter)
         .Load_Key_Binding(KeyStop)
         .Load_Key_Binding(KeyGuard)
         .Load_Key_Binding(KeyNext)
         .Load_Key_Binding(KeyPrevious)
         .Load_Key_Binding(KeyFormation)
         .Load_Key_Binding(KeyHome1)
         .Load_Key_Binding(KeyHome2)
         .Load_Key_Binding(KeyBase)
         .Load_Key_Binding(KeyResign)
         .Load_Key_Binding(KeyAlliance)
         .Load_Key_Binding(KeyBookmark1)
         .Load_Key_Binding(KeyBookmark2)
         .Load_Key_Binding(KeyBookmark3)
         .Load_Key_Binding(KeyBookmark4)
         .Load_Key_Binding(KeySelectView)
         .Load_Key_Binding(KeyRepair)
         .Load_Key_Binding(KeyRepairOn)
         .Load_Key_Binding(KeyRepairOff)
         .Load_Key_Binding(KeySell)
         .Load_Key_Binding(KeySellOn)
         .Load_Key_Binding(KeySellOff)
         .Load_Key_Binding(KeyMap)
         .Load_Key_Binding(KeySidebarUp)
         .Load_Key_Binding(KeySidebarDown)
         .Load_Key_Binding(KeyOption1)
         .Load_Key_Binding(KeyOption2)
         .Load_Key_Binding(KeyScrollLeft)
         .Load_Key_Binding(KeyScrollRight)
         .Load_Key_Binding(KeyScrollUp)
         .Load_Key_Binding(KeyScrollDown)
         .Load_Key_Binding(KeyQueueMove1)
         .Load_Key_Binding(KeyQueueMove2)
         .Load_Key_Binding(KeyTeam1)
         .Load_Key_Binding(KeyTeam2)
         .Load_Key_Binding(KeyTeam3)
         .Load_Key_Binding(KeyTeam4)
         .Load_Key_Binding(KeyTeam5)
         .Load_Key_Binding(KeyTeam6)
         .Load_Key_Binding(KeyTeam7)
         .Load_Key_Binding(KeyTeam8)
         .Load_Key_Binding(KeyTeam9)
         .Load_Key_Binding(KeyTeam10)
         .Load_Key_Binding(KeyDeploy);

        /*
        **	Check for and possible enable true object names.
        */
        c.Load_Special_Setting("TrueNames", PARM_TRUENAME, IsNamed, true);

        /*
        **	Enable 6 player games if special flag is detected.
        */
        c.Load_Obfusticated_Setting("Players", PARM_6PLAYER, MPlayerMax, 6);

        /*
        **	Enable three point turning logic as indicated.
        */
        c.Load_Special_Setting("Rotation", PARM_3POINT, IsThreePoint, true);

        /*
        **	Allow purchase of the helipad separately from the helicopter.
        */
        c.Load_Special_Setting("Helipad", PARM_HELIPAD, IsSeparate, true);

        /*
        **	Allow the MCV to undeploy rather than sell.
        */
        c.Load_Special_Setting("MCV", PARM_MCV, IsMCVDeploy, true);

        /*
        **	Allow disabling of building bibs so that tigher building packing can occur.
        */
        c.Load_Special_Setting("Bibs", PARM_BIB, IsRoad, true);

        /*
        **	Allow targeting of trees without having to hold down the shift key.
        */
        c.Load_Special_Setting("TreeTarget", PARM_TREETARGET, IsTreeTarget, true);

        /*
        **	Allow infantry to fire while moving. Attacker gets advantage with this flag.
        */
        c.Load_Special_Setting("Combat", PARM_COMBAT, IsDefenderAdvantage, false);

        /*
        **	Allow custom scores.
        */
        c.Load_Special_Setting("Scores", PARM_SCORE, IsVariation, true);

        /*
        **	Smarter self defense logic. Tanks will try to run over adjacent infantry. Buildings
        **	will automatically return fire if they are fired upon. Infantry will run from an
        **	incoming explosive (grenade or napalm) or damage that can't be directly addressed.
        */
        c.Load_Special_Setting("CombatIQ", PARM_IQ, IsSmartDefense, true);
        c.Load_Special_Setting("CombatIQ", PARM_IQ, IsScatter, true);

        /*
        **	Enable the infantry squish marks when run over by a vehicle.
        */
        c.Load_Special_Setting("Overrun", PARM_SQUISH, IsGross, true);

        /*
        **	Enable the human generated sound effects.
        */
        c.Load_Special_Setting("Sounds", PARM_HUMAN, IsJuvenile, true);

        /*
        **	Scrolling is disabled over the tabs with this option.
        */
        c.Load_Special_Setting("Scrolling", PARM_SCROLLING, IsScrollMod, true);
    });

    Set_Brightness(Brightness);
    Set_Sound_Volume(Volume);
    Set_Score_Volume(ScoreVolume);
    Set_Contrast(Contrast);
    Set_Color(Color);
    Set_Tint(Tint);
    Set_Repeat(IsScoreRepeat);
    Set_Shuffle(IsScoreShuffle);
}

/***********************************************************************************************
 * OptionsClass::Update -- writes option values to common settings sections                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *=============================================================================================*/
void OptionsClass::Update()
{
    /*
    **	Save Options settings
    */
    Get_Options_Section()
        .Set_Int_Var(GameSpeed)
        .Set_Int_Var(ScrollRate)
        .Set_Int_Var(Brightness)
        .Set_Int_Var(Volume)
        .Set_Int_Var(ScoreVolume)
        .Set_Int_Var(Contrast)
        .Set_Int_Var(Color)
        .Set_Int_Var(Tint)
        .Set_Bool_Var(AutoScroll)
        .Set_Bool_Var(IsScoreRepeat)
        .Set_Bool_Var(IsScoreShuffle)
        .Set("DeathAnnounce", static_cast<bool>(IsDeathAnnounce))
        .Set("FreeScrolling", static_cast<bool>(IsFreeScroll))
        .Set_Bool_Var(SkipExpansionCdCheck)
        .Set_With_TdConverter(KeyForceMove1)
        .Set_With_TdConverter(KeyForceMove2)
        .Set_With_TdConverter(KeyForceAttack1)
        .Set_With_TdConverter(KeyForceAttack2)
        .Set_With_TdConverter(KeySelect1)
        .Set_With_TdConverter(KeySelect2)
        .Set_With_TdConverter(KeyScatter)
        .Set_With_TdConverter(KeyStop)
        .Set_With_TdConverter(KeyGuard)
        .Set_With_TdConverter(KeyNext)
        .Set_With_TdConverter(KeyPrevious)
        .Set_With_TdConverter(KeyFormation)
        .Set_With_TdConverter(KeyHome1)
        .Set_With_TdConverter(KeyHome2)
        .Set_With_TdConverter(KeyBase)
        .Set_With_TdConverter(KeyResign)
        .Set_With_TdConverter(KeyAlliance)
        .Set_With_TdConverter(KeyBookmark1)
        .Set_With_TdConverter(KeyBookmark2)
        .Set_With_TdConverter(KeyBookmark3)
        .Set_With_TdConverter(KeyBookmark4)
        .Set_With_TdConverter(KeySelectView)
        .Set_With_TdConverter(KeyRepair)
        .Set_With_TdConverter(KeyRepairOn)
        .Set_With_TdConverter(KeyRepairOff)
        .Set_With_TdConverter(KeySell)
        .Set_With_TdConverter(KeySellOn)
        .Set_With_TdConverter(KeySellOff)
        .Set_With_TdConverter(KeyMap)
        .Set_With_TdConverter(KeySidebarUp)
        .Set_With_TdConverter(KeySidebarDown)
        .Set_With_TdConverter(KeyOption1)
        .Set_With_TdConverter(KeyOption2)
        .Set_With_TdConverter(KeyScrollLeft)
        .Set_With_TdConverter(KeyScrollRight)
        .Set_With_TdConverter(KeyScrollUp)
        .Set_With_TdConverter(KeyScrollDown)
        .Set_With_TdConverter(KeyQueueMove1)
        .Set_With_TdConverter(KeyQueueMove2)
        .Set_With_TdConverter(KeyTeam1)
        .Set_With_TdConverter(KeyTeam2)
        .Set_With_TdConverter(KeyTeam3)
        .Set_With_TdConverter(KeyTeam4)
        .Set_With_TdConverter(KeyTeam5)
        .Set_With_TdConverter(KeyTeam6)
        .Set_With_TdConverter(KeyTeam7)
        .Set_With_TdConverter(KeyTeam8)
        .Set_With_TdConverter(KeyTeam9)
        .Set_With_TdConverter(KeyTeam10)
        .Set_With_TdConverter(KeyDeploy)
        .Set_With_TdConverter(KeySelectAllOfType);
}

/***********************************************************************************************
 * OptionsClass::Set -- Sets options based on current settings                                 *
 *                                                                                             *
 * Use this routine to adjust the palette or sound settings after a fresh scenario load.       *
 * It assumes the values needed are already loaded into OptionsClass.                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/24/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
void OptionsClass::Set(void)
{
    Set_Brightness(Brightness);
    Set_Contrast(Contrast);
    Set_Color(Color);
    Set_Tint(Tint);
    Set_Sound_Volume(Volume, false);
    Set_Score_Volume(ScoreVolume);
    Set_Repeat(IsScoreRepeat);
    Set_Shuffle(IsScoreShuffle);
}

/***********************************************************************************************
 * OptionsClass::Normalize_Delay -- Normalizes delay factor to keep rate constant.             *
 *                                                                                             *
 *    This routine is used to adjust delay factors that MUST be synchronized on all machines   *
 *    but should maintain a speed as close to constant as possible. Building animations are    *
 *    a good example of this.                                                                  *
 *                                                                                             *
 * INPUT:   delay -- The normal delay factor.                                                  *
 *                                                                                             *
 * OUTPUT:  Returns with the delay to use that has been modified so that a reasonably constant *
 *          rate will result.                                                                  *
 *                                                                                             *
 * WARNINGS:   This calculation is crude due to the coarse resolution that a 1/15 second timer *
 *             allows.                                                                         *
 *                                                                                             *
 *             Use of this routine ASSUMES that the GameSpeed is synchronized on all machines. *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/18/1995 JLB : Created.                                                                 *
 *   06/30/1995 JLB : Handles low values in a more consistent manner.                          *
 *=============================================================================================*/
int OptionsClass::Normalize_Delay(int delay) const
{
    static int _adjust[][8] = {
        {2, 2, 1, 1, 1, 1, 1, 1}, {3, 3, 3, 2, 2, 2, 1, 1}, {5, 4, 4, 3, 3, 2, 2, 1}, {7, 6, 5, 4, 4, 4, 3, 2}};
    if (delay) {
        if (delay < 5) {
            delay = _adjust[delay - 1][GameSpeed];
        } else {
            delay = ((delay * 8) / (GameSpeed + 1));
        }
    }
    return (delay);
}

void OptionsClass::Fixup_Palette(void) const
{
    Adjust_Palette(OriginalPalette, GamePalette, Brightness, Color, Tint, Contrast);
}

int OptionsClass::Normalize_Sound(int volume) const
{
    return (Fixed_To_Cardinal(volume, Volume));
}

RuleSection& OptionsClass::Get_Options_Section() const
{
    if (CommonSettings == nullptr) {
        throw std::runtime_error("Attempted to read common settings before OptionsClass::One_Time was called");
    }

    return CommonSettings->Get_Sections()["Options"];
}
