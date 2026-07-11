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

/* $Header:   F:\projects\c&c\vcs\code\special.cpv   1.4   16 Oct 1995 16:50:06   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SPECIAL.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 05/27/95                                                     *
 *                                                                                             *
 *                  Last Update : May 27, 1995 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "function.h"
#include "common/framelimit.h"

void Special_Dialog(void)
{
    SpecialClass oldspecial = Special;
    GadgetClass* buttons = NULL;
    static struct
    {
        int Description;
        int Setting;
        CheckBoxClass* Button;
    } _options[] = {
        //		{TXT_DEFENDER_ADVANTAGE, 0, 0},
        {TXT_SEPARATE_HELIPAD, 0, 0},
        {TXT_VISIBLE_TARGET, 0, 0},
        {TXT_TREE_TARGET, 0, 0},
        {TXT_MCV_DEPLOY, 0, 0},
        {TXT_SMART_DEFENCE, 0, 0},
        {TXT_THREE_POINT, 0, 0},
        //		{TXT_TIBERIUM_GROWTH, 0, 0},
        //		{TXT_TIBERIUM_SPREAD, 0, 0},
        {TXT_TIBERIUM_FAST, 0, 0},
        {TXT_ROAD_PIECES, 0, 0},
        {TXT_SCATTER, 0, 0},
        {TXT_SHOW_NAMES, 0, 0},
    };

    const auto factor = Get_Resolution_Factor();
    const auto option_width = factor == 0 ? 236 : 320;
    const auto option_height = factor == 0 ? 162 : 220;
    const auto option_x = (Try_Get_Resolution_Mode_Width().value_or(SeenBuff.Get_Width()) - option_width) / 2;
    const auto option_y = (Try_Get_Resolution_Mode_Height().value_or(SeenBuff.Get_Height()) - option_height) / 2;

    TextButtonClass ok(200, TXT_OK, TPF_6PT_GRAD | TPF_NOSHADOW, option_x + (factor == 0 ? 5 : 10), option_y + option_height - (factor == 0 ? 15 : 25));
    TextButtonClass cancel(
        201, TXT_CANCEL, TPF_6PT_GRAD | TPF_NOSHADOW, option_x + option_width - (factor == 0 ? 50 : 55), option_y + option_height - (factor == 0 ? 15: 25));
    buttons = &ok;
    cancel.Add(*buttons);
    int index;
    for (index = 0; index < sizeof(_options) / sizeof(_options[0]); index++) {
        _options[index].Button = new CheckBoxClass(100 + index, option_x + (factor == 0 ? 7 : 10), option_y + (factor == 0 ? 20 : 30) + (index * (factor == 0 ? 10 : 16)));
        if (_options[index].Button) {
            _options[index].Button->Add(*buttons);

            bool value = false;
            switch (_options[index].Description) {
            case TXT_SEPARATE_HELIPAD:
                value = Special.IsSeparate;
                break;

            case TXT_SHOW_NAMES:
                value = Special.IsNamed;
                break;

            case TXT_DEFENDER_ADVANTAGE:
                value = Special.IsDefenderAdvantage;
                break;

            case TXT_VISIBLE_TARGET:
                value = Special.IsVisibleTarget;
                break;

            case TXT_TREE_TARGET:
                value = Special.IsTreeTarget;
                break;

            case TXT_MCV_DEPLOY:
                value = Special.IsMCVDeploy;
                break;

            case TXT_SMART_DEFENCE:
                value = Special.IsSmartDefense;
                break;

            case TXT_THREE_POINT:
                value = Special.IsThreePoint;
                break;

            case TXT_TIBERIUM_GROWTH:
                value = Special.IsTGrowth;
                break;

            case TXT_TIBERIUM_SPREAD:
                value = Special.IsTSpread;
                break;

            case TXT_TIBERIUM_FAST:
                value = Special.IsTFast;
                break;

            case TXT_ROAD_PIECES:
                value = Special.IsRoad;
                break;

            case TXT_SCATTER:
                value = Special.IsScatter;
                break;
            }

            _options[index].Setting = value;
            if (value) {
                _options[index].Button->Turn_On();
            } else {
                _options[index].Button->Turn_Off();
            }
        }
    }

    Map.Override_Mouse_Shape(MOUSE_NORMAL);
    Set_Logic_Page(SeenBuff);
    bool recalc = true;
    bool display = true;
    bool process = true;
    while (process) {

        /*
        ** If we have just received input focus again after running in the background then
        ** we need to redraw.
        */
        if (AllSurfaces.SurfacesRestored) {
            AllSurfaces.SurfacesRestored = false;
            display = true;
        }

        if (GameToPlay == GAME_NORMAL) {
            Call_Back();
        } else {
            if (Main_Loop()) {
                process = false;
            }
        }

        if (display) {
            display = false;

            Hide_Mouse();
            Dialog_Box(option_x, option_y, option_width, option_height);
            Draw_Caption(TXT_SPECIAL_OPTIONS, option_x, option_y, option_width);

            for (index = 0; index < sizeof(_options) / sizeof(_options[0]); index++) {
                Fancy_Text_Print(_options[index].Description,
                                 _options[index].Button->X + (factor == 0 ? 10 : 13),
                                 _options[index].Button->Y + (factor == 0 ? 0 : -2),
                                 CC_GREEN,
                                 TBLACK,
                                 TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
            }
            buttons->Draw_All();
            Show_Mouse();
        }

        KeyNumType input = buttons->Input();
        switch (input) {
        case KN_ESC:
        case 200 | KN_BUTTON:
            process = false;
            for (index = 0; index < sizeof(_options) / sizeof(_options[0]); index++) {
                switch (_options[index].Description) {
                case TXT_SEPARATE_HELIPAD:
                    oldspecial.IsSeparate = _options[index].Setting;
                    break;

                case TXT_SHOW_NAMES:
                    oldspecial.IsNamed = _options[index].Setting;
                    break;

                case TXT_DEFENDER_ADVANTAGE:
                    oldspecial.IsDefenderAdvantage = _options[index].Setting;
                    break;

                case TXT_VISIBLE_TARGET:
                    oldspecial.IsVisibleTarget = _options[index].Setting;
                    break;

                case TXT_TREE_TARGET:
                    oldspecial.IsTreeTarget = _options[index].Setting;
                    break;

                case TXT_MCV_DEPLOY:
                    oldspecial.IsMCVDeploy = _options[index].Setting;
                    break;

                case TXT_SMART_DEFENCE:
                    oldspecial.IsSmartDefense = _options[index].Setting;
                    break;

                case TXT_THREE_POINT:
                    oldspecial.IsThreePoint = _options[index].Setting;
                    break;

                case TXT_TIBERIUM_GROWTH:
                    oldspecial.IsTGrowth = _options[index].Setting;
                    break;

                case TXT_TIBERIUM_SPREAD:
                    oldspecial.IsTSpread = _options[index].Setting;
                    break;

                case TXT_TIBERIUM_FAST:
                    oldspecial.IsTFast = _options[index].Setting;
                    break;

                case TXT_ROAD_PIECES:
                    oldspecial.IsRoad = _options[index].Setting;
                    break;

                case TXT_SCATTER:
                    oldspecial.IsScatter = _options[index].Setting;
                    break;
                }
            }
            OutList.Add(EventClass(oldspecial));
            break;

        case 201 | KN_BUTTON:
            process = false;
            break;

        case KN_NONE:
            break;

        default:
            index = (input & ~KN_BUTTON) - 100;
            if ((unsigned)index < sizeof(_options) / sizeof(_options[0])) {
                _options[index].Setting = (_options[index].Setting == false);
                if (_options[index].Setting) {
                    _options[index].Button->Turn_On();
                } else {
                    _options[index].Button->Turn_Off();
                }
            }
            break;
        }
    }

    Map.Revert_Mouse_Shape();
    HiddenPage.Clear();
    Map.Flag_To_Redraw(true);
    Map.Render();
}

int Fetch_Difficulty(void)
{
    static const char TXT_EASY[] = "Easy";
    static const char TXT_NORMAL[] = "Normal";
    static const char TXT_HARD[] = "Hard";
    static const char TXT_DIFFICULTY[] = "Difficulty";

    int factor = (SeenBuff.Get_Width() == 320) ? 1 : 2;
    int const w = 250 * factor;
    int const h = 70 * factor;
    int const x = (Try_Get_Resolution_Mode_Width().value_or(SeenBuff.Get_Width()) / 2) - w / 2;
    int const y = (Try_Get_Resolution_Mode_Height().value_or(SeenBuff.Get_Height()) / 2) - h / 2;
    int const bwidth = 40 * factor;

    /*
    **	Fill the description buffer with the description text. Break
    **	the text into appropriate spacing.
    */
    char buffer[512];
    strncpy(buffer, TXT_DIFFICULTY, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    Fancy_Text_Print(TXT_NONE, 0, 0, CC_GREEN, TBLACK, TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
    int width;
    int height;
    Format_Window_String(buffer, w - 60 * factor, width, height);

    /*
    **	Create the OK button.
    */
    TextButtonClass okbutton(1, TXT_OK, TPF_BUTTON, (x + w) - (bwidth + 20 * factor), (y + h) - (18 * factor), bwidth);
    TextButtonClass cancelbutton(3, TXT_CANCEL, TPF_BUTTON, x + (20 * factor), (y + h) - (18 * factor), bwidth);
    GadgetClass* buttonlist = &okbutton;
    cancelbutton.Add(*buttonlist);

    /*
    **	Create the slider button.
    */
    SliderClass slider(2, x + 20 * factor, y + h - 38 * factor, w - 40 * factor, 8 * factor, true);
    if (Rule.IsFineDifficulty) {
        slider.Set_Maximum(5);
        slider.Set_Value(2);
    } else {
        slider.Set_Maximum(3);
        slider.Set_Value(1);
    }
    slider.Add(*buttonlist);

    /*
    **	Main Processing Loop.
    */
    Set_Logic_Page(SeenBuff);
    bool redraw = true;
    bool process = true;
    while (process) {

        if (redraw) {
            redraw = false;

            /*
            **	Draw the background of the dialog.
            */
            Hide_Mouse();
            Dialog_Box(x, y, w, h);
            Draw_Caption(TXT_NONE, x, y, w);

            /*
            **	Draw the body of the message.
            */
            Fancy_Text_Print(buffer,
                             w / 2 + x,
                             5 * factor + y,
                             CC_GREEN,
                             TBLACK,
                             TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            /*
            **	Display the descripton of the slider range.
            */
            Fancy_Text_Print(TXT_HARD,
                             slider.X + slider.Width,
                             slider.Y - 9 * factor,
                             CC_GREEN,
                             TBLACK,
                             TPF_RIGHT | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_DROPSHADOW);
            Fancy_Text_Print(TXT_EASY,
                             slider.X,
                             slider.Y - 9 * factor,
                             CC_GREEN,
                             TBLACK,
                             TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_DROPSHADOW);
            Fancy_Text_Print(TXT_NORMAL,
                             slider.X + (slider.Width / 2),
                             slider.Y - 9 * factor,
                             CC_GREEN,
                             TBLACK,
                             TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_DROPSHADOW);

            /*
            **	Redraw the buttons.
            */
            if (buttonlist) {
                buttonlist->Draw_All();
            }
            Show_Mouse();
        }

        /*
        **	Invoke game callback.
        */
        Call_Back();

        /*
        ** Handle possible surface loss due to a focus switch
        */
        if (AllSurfaces.SurfacesRestored) {
            AllSurfaces.SurfacesRestored = false;
            redraw = true;
            continue;
        }

        /*
        **	Fetch and process input.
        */
        KeyNumType input = buttonlist->Input();

        switch (input) {
        case KN_RETURN:
        case (1 | BUTTON_FLAG):
            process = false;
            break;

        case (3 | BUTTON_FLAG):
            return -1;

        default:
            break;
        }

        Frame_Limiter();
    }

    return (slider.Get_Value() * (Rule.IsFineDifficulty ? 1 : 2));
}

void SpecialClass::Init()
{
    IsScrollMod = false;
    IsGross = false;
    IsEasy = false;
    IsDifficult = false;
    IsSpeedBuild = false;
    IsDefenderAdvantage = true;
    IsVisibleTarget = false;
    IsVariation = false;
    IsJurassic = false;
    IsJuvenile = false;
    IsSmartDefense = false;
    IsTreeTarget = false;
    IsMCVDeploy = false;
    IsVisceroids = false;
    IsMonoEnabled = false;
    IsInert = false;
    IsShowPath = false;
    IsThreePoint = false;
    IsTGrowth = true;
    IsTSpread = true;
    IsTFast = true;
    IsRoad = false;
    IsScatter = false;
    IsCaptureTheFlag = false;
    IsNamed = false;
    IsFromInstall = false;
    IsSeparate = false;
    IsEarlyWin = false;
    HealthBarDisplayMode = HB_SELECTED;
    ResourceBarDisplayMode = RB_SELECTED;
    ModernBalance = false;

    if (RuleOverrides != nullptr) {
        delete RuleOverrides;
    }

    RuleOverrides = new RuleSections();
}

TO_JSON(SpecialClass)
{
    BITFIELD_TO_JSON(IsEasy);
    BITFIELD_TO_JSON(IsDifficult);
    BITFIELD_TO_JSON(IsSpeedBuild);
    BITFIELD_TO_JSON(IsSeparate);
    BITFIELD_TO_JSON(IsDefenderAdvantage);
    BITFIELD_TO_JSON(IsNamed);
    BITFIELD_TO_JSON(IsFromInstall);
    BITFIELD_TO_JSON(IsCaptureTheFlag);
    BITFIELD_TO_JSON(IsVisibleTarget);
    BITFIELD_TO_JSON(IsJuvenile);
    BITFIELD_TO_JSON(IsSmartDefense);
    BITFIELD_TO_JSON(IsTreeTarget);
    BITFIELD_TO_JSON(IsMCVDeploy);
    BITFIELD_TO_JSON(IsVisceroids);
    BITFIELD_TO_JSON(IsMonoEnabled);
    BITFIELD_TO_JSON(IsInert);
    BITFIELD_TO_JSON(IsShowPath);
    BITFIELD_TO_JSON(IsThreePoint);
    BITFIELD_TO_JSON(IsTGrowth);
    BITFIELD_TO_JSON(IsTSpread);
    BITFIELD_TO_JSON(IsTFast);
    BITFIELD_TO_JSON(IsRoad);
    BITFIELD_TO_JSON(IsScatter);
    BITFIELD_TO_JSON(IsJurassic);
    BITFIELD_TO_JSON(IsVariation);
    BITFIELD_TO_JSON(IsGross);
    BITFIELD_TO_JSON(IsScrollMod);
    BITFIELD_TO_JSON(IsEarlyWin);
    FIELD_VALUE_TO_JSON(HealthBarDisplayMode, static_cast<int>(p.HealthBarDisplayMode));
    FIELD_VALUE_TO_JSON(ResourceBarDisplayMode, static_cast<int>(p.ResourceBarDisplayMode));
    BITFIELD_TO_JSON(ModernBalance);

    if (p.RuleOverrides != nullptr) {
        FIELD_VALUE_TO_JSON(RuleOverrides, *p.RuleOverrides);
    } else {
        j[NAMEOF(RuleOverrides)] = nlohmann::json::object();
    }
}

FROM_JSON(SpecialClass)
{
    p.Init();

    BITFIELD_FROM_JSON(IsEasy);
    BITFIELD_FROM_JSON(IsDifficult);
    BITFIELD_FROM_JSON(IsSpeedBuild);
    BITFIELD_FROM_JSON(IsSeparate);
    BITFIELD_FROM_JSON(IsDefenderAdvantage);
    BITFIELD_FROM_JSON(IsNamed);
    BITFIELD_FROM_JSON(IsFromInstall);
    BITFIELD_FROM_JSON(IsCaptureTheFlag);
    BITFIELD_FROM_JSON(IsVisibleTarget);
    BITFIELD_FROM_JSON(IsJuvenile);
    BITFIELD_FROM_JSON(IsSmartDefense);
    BITFIELD_FROM_JSON(IsTreeTarget);
    BITFIELD_FROM_JSON(IsMCVDeploy);
    BITFIELD_FROM_JSON(IsVisceroids);
    BITFIELD_FROM_JSON(IsMonoEnabled);
    BITFIELD_FROM_JSON(IsInert);
    BITFIELD_FROM_JSON(IsShowPath);
    BITFIELD_FROM_JSON(IsThreePoint);
    BITFIELD_FROM_JSON(IsTGrowth);
    BITFIELD_FROM_JSON(IsTSpread);
    BITFIELD_FROM_JSON(IsTFast);
    BITFIELD_FROM_JSON(IsRoad);
    BITFIELD_FROM_JSON(IsScatter);
    BITFIELD_FROM_JSON(IsJurassic);
    BITFIELD_FROM_JSON(IsVariation);
    BITFIELD_FROM_JSON(IsGross);
    BITFIELD_FROM_JSON(IsScrollMod);
    BITFIELD_FROM_JSON(IsEarlyWin);

    // HealthBarDisplayMode
    int healthBarDisplayMode;
    FIELD_FROM_JSON_TO_VALUE(HealthBarDisplayMode, healthBarDisplayMode);

    if (
        healthBarDisplayMode < SpecialClass::eHealthBarDisplayMode::HB_DAMAGED
        || healthBarDisplayMode > SpecialClass::eHealthBarDisplayMode::HB_SELECTED
    ) {
        throw CncJsonException("Invalid HealthBarDisplayMode field value");
    }

    p.HealthBarDisplayMode = static_cast<SpecialClass::eHealthBarDisplayMode>(
        healthBarDisplayMode
    );

    // ResourceBarDisplayMode
    int resourceBarDisplayMode;
    FIELD_FROM_JSON_TO_VALUE(ResourceBarDisplayMode, resourceBarDisplayMode);

    if (
        resourceBarDisplayMode < SpecialClass::eResourceBarDisplayMode::RB_SELECTED
        || resourceBarDisplayMode > SpecialClass::eResourceBarDisplayMode::RB_ALWAYS
    ) {
        throw CncJsonException("Invalid ResourceBarDisplayMode field value");
    }

    p.ResourceBarDisplayMode = static_cast<SpecialClass::eResourceBarDisplayMode>(
        resourceBarDisplayMode
    );

    BITFIELD_FROM_JSON(ModernBalance);

    if (p.RuleOverrides == nullptr) {
        throw CncJsonException("Attempted to deserialize SpecialClass from json when RuleOverrides was nullptr");
    }

    p.RuleOverrides->Clear();
    from_json(j[NAMEOF(RuleOverrides)], *p.RuleOverrides);
}
