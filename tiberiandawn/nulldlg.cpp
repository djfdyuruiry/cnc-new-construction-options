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

/* $Header:   F:\projects\c&c\vcs\code\nulldlg.cpv   1.9   16 Oct 1995 16:52:12   JOE_BOSTIC  $ */
/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S        **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : NULLDLG.CPP                              *
 *                                                                         *
 *                   Programmer : Bill R. Randolph                         *
 *                                                                         *
 *                   Start Date : 04/29/95                                 *
 *                                                                         *
 *                  Last Update : April 29, 1995 [BRR]                     *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 *   Com_Scenario_Dialog -- Skirmish game scenario selection dialog        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#include "function.h"
#include "drop.h"
#include "framelimit.h"

struct ControlDimension
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
};

class SkirmishScenarioDialog final
{
    /*........................................................................
    Button Enumerations
    ........................................................................*/
    typedef enum
    {
        BUTTON_NAME = 100,
        BUTTON_HOUSE,
        BUTTON_AI_DIFF_1,
        BUTTON_AI_DIFF_2,
        BUTTON_AI_DIFF_3,
        BUTTON_AI_DIFF_4,
        BUTTON_AI_DIFF_5,
        BUTTON_AI_HOUSE_1,
        BUTTON_AI_HOUSE_2,
        BUTTON_AI_HOUSE_3,
        BUTTON_AI_HOUSE_4,
        BUTTON_AI_HOUSE_5,
        BUTTON_CREDITS,
        BUTTON_TIBERIUMSCALE,
        BUTTON_OPTIONS,
        BUTTON_SCENARIOLIST,
        BUTTON_COUNT,
        BUTTON_LEVEL,
        BUTTON_OK,
        BUTTON_LOAD,
        BUTTON_CANCEL,
        BUTTON_DIFFICULTY,
        BUTTON_COLOR_1,
        BUTTON_COLOR_2,
        BUTTON_COLOR_3,
        BUTTON_COLOR_4,
        BUTTON_COLOR_5,
        BUTTON_COLOR_6,
        BUTTON_PLAYERLIST // not used
    } ButtonType;

    /*........................................................................
    Redraw values: in order from "top" to "bottom" layer of the dialog
    ........................................................................*/
    typedef enum
    {
        REDRAW_NONE = 0,
        REDRAW_MESSAGE,
        REDRAW_COLORS,
        REDRAW_BUTTONS,
        REDRAW_BACKGROUND,
        REDRAW_ALL = REDRAW_BACKGROUND
    } RedrawType;

    static constexpr auto DropdownTextLength = 25;
    static inline int OptionTabs[] = {8};

    // dimensions
    int Factor;

    int X;
    int Y;
    int Width;
    int Height;
    int Center;

    int TextHeight;
    int MarginWidth;
    int MarginHeight;

    // button shapes
    void const* UpButtonShape;
    void const* DownButtonShape;

    std::map<ButtonType, ControlDimension> Dimensions;
    std::map<ButtonType, std::unique_ptr<char[]>> Text;
    std::map<ButtonType, std::unique_ptr<GadgetClass>> Controls;
    GadgetClass* CommandChain;

    template<class T>
    T& Get_Control(const ButtonType type)
    {
        return *reinterpret_cast<T*>(Controls[type].get());
    }

    void Render(RedrawType& display)
    {
        if (!display) {
            return;
        }
        char txt[80];

        Hide_Mouse();
        /*
        .................. Redraw backgound & dialog box ...................
        */
        if (display >= REDRAW_BACKGROUND) {
            Dialog_Box(X, Y, Width, Height);

            // init font variables

            Fancy_Text_Print(
                TXT_NONE, 0, 0, TBLACK, TBLACK, TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            /*...............................................................
            Dialog & Field labels
            ...............................................................*/
#ifdef FORCE_WINSOCK
            if (Winsock.Get_Connected()) {
                Draw_Caption(TXT_HOST_INTERNET_GAME, X, Y, Width);
            } else {
                Draw_Caption(TXT_HOST_SERIAL_GAME, X, Y, Width);
            }
#else
            Draw_Caption(TXT_NONE, X, Y, Width);
#endif // FORCE_WINSOCK

            Fancy_Text_Print(TXT_YOUR_NAME,
                             Dimensions[BUTTON_NAME].x + (Dimensions[BUTTON_NAME].w / 2),
                             Dimensions[BUTTON_NAME].y - TextHeight - (1 * Factor),
                             CC_GREEN,
                             TBLACK,
                             TPF_RIGHT | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            Fancy_Text_Print(TXT_SIDE_COLON,
                             Dimensions[BUTTON_HOUSE].x + (Dimensions[BUTTON_HOUSE].w / 2),
                             Dimensions[BUTTON_HOUSE].y - TextHeight - (1 * Factor),
                             CC_GREEN,
                             TBLACK,
                             TPF_RIGHT | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            Fancy_Text_Print(TXT_COLOR_COLON,
                             X + ((Width / 4) * 3),
                             Dimensions[BUTTON_COLOR_1].y - TextHeight - (1 * Factor),
                             CC_GREEN,
                             TBLACK,
                             TPF_RIGHT | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            auto& difficulty = Get_Control<SliderClass>(BUTTON_DIFFICULTY);

            Fancy_Text_Print("Easy", // TODO: Locale file entry
                             difficulty.X,
                             difficulty.Y - 8 * Factor,
                             CC_GREEN,
                             TBLACK,
                             TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            Fancy_Text_Print("Hard", // TODO: Locale file entry
                             difficulty.X + difficulty.Width,
                             difficulty.Y - 8 * Factor,
                             CC_GREEN,
                             TBLACK,
                             TPF_RIGHT | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            Fancy_Text_Print("Normal", // TODO: Locale file entry
                             difficulty.X + difficulty.Width / 2,
                             difficulty.Y - 8 * Factor,
                             CC_GREEN,
                             TBLACK,
                             TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            Fancy_Text_Print(TXT_SCENARIOS,
                             Dimensions[BUTTON_SCENARIOLIST].x + (Dimensions[BUTTON_SCENARIOLIST].w / 2),
                             Dimensions[BUTTON_SCENARIOLIST].y - TextHeight - (1 * Factor),
                             CC_GREEN,
                             TBLACK,
                             TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            Fancy_Text_Print(TXT_COUNT,
                             Dimensions[BUTTON_COUNT].x - 3 * Factor,
                             Dimensions[BUTTON_COUNT].y,
                             CC_GREEN,
                             TBLACK,
                             TPF_NOSHADOW | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_RIGHT);

            Fancy_Text_Print(TXT_LEVEL,
                             Dimensions[BUTTON_LEVEL].x - 3 * Factor,
                             Dimensions[BUTTON_LEVEL].y,
                             CC_GREEN,
                             TBLACK,
                             TPF_NOSHADOW | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_RIGHT);

            Fancy_Text_Print(TXT_START_CREDITS_COLON,
                             Dimensions[BUTTON_CREDITS].x - 3 * Factor,
                             Dimensions[BUTTON_CREDITS].y,
                             CC_GREEN,
                             TBLACK,
                             TPF_RIGHT | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            Fancy_Text_Print("Tiberium Growth:", // TODO: Locale file entry
                             Dimensions[BUTTON_TIBERIUMSCALE].x - 3 * Factor,
                             Dimensions[BUTTON_TIBERIUMSCALE].y,
                             CC_GREEN,
                             TBLACK,
                             TPF_RIGHT | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            // AI player setting headers
            Fancy_Text_Print("Player", // TODO: Locale file entry
                             (Dimensions[BUTTON_AI_HOUSE_1].x
                                - static_cast<int>(nearbyint(Dimensions[BUTTON_AI_HOUSE_1].w * 1.5)) - (10 * Factor))
                                + (static_cast<int>((Dimensions[BUTTON_AI_HOUSE_1].w * 1.5) / 1.25)),
                             Dimensions[BUTTON_AI_HOUSE_1].y - TextHeight - (2 * Factor),
                             CC_GREEN,
                             TBLACK,
                             TPF_RIGHT | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            Fancy_Text_Print("Side", // TODO: Locale file entry
                             Dimensions[BUTTON_AI_HOUSE_1].x
                                + static_cast<int>(nearbyint(Dimensions[BUTTON_AI_HOUSE_1].w / 1.25)),
                             Dimensions[BUTTON_AI_HOUSE_1].y - TextHeight - (2 * Factor),
                             CC_GREEN,
                             TBLACK,
                             TPF_RIGHT | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            // AI player difficulty and house checkbox labels
            const auto cur_ai_house_label_x = (Dimensions[BUTTON_AI_HOUSE_1].x
                - static_cast<int>(nearbyint(Dimensions[BUTTON_AI_HOUSE_1].w * 1.5))
                - (10 * Factor)) - 3 * Factor;

            for (auto control = BUTTON_AI_DIFF_1; control <= BUTTON_AI_DIFF_5; ++control) {
                Fancy_Text_Print(std::format("AI {}:", control - BUTTON_HOUSE).c_str(), // TODO: Locale file entry
                                 cur_ai_house_label_x,
                                 Dimensions[control].y,
                                 CC_GREEN,
                                 TBLACK,
                                 TPF_RIGHT | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
            }
        }

        /*..................................................................
        Draw the color boxes
        ..................................................................*/
        if (display >= REDRAW_COLORS) {
            for (auto control = BUTTON_COLOR_1; control <= BUTTON_COLOR_6; ++control) {
                const auto mplayer_idx = control - BUTTON_COLOR_1;

                LogicPage->Fill_Rect(Dimensions[control].x + 1 * Factor,
                                     Dimensions[control].y + 1 * Factor,
                                     Dimensions[control].x + 1 * Factor + Dimensions[control].w - 2 * Factor,
                                     Dimensions[control].y + 1 * Factor + Dimensions[control].h - 2 * Factor,
                                     MPlayerGColors[mplayer_idx]);

                Draw_Box(
                    Dimensions[control].x,
                    Dimensions[control].y,
                    Dimensions[control].w,
                    Dimensions[control].h,
                    mplayer_idx == MPlayerColorIdx ? BOXSTYLE_GREEN_DOWN : BOXSTYLE_GREEN_RAISED,
                    false
                );
            }
        }

        /*..................................................................
        Draw the message:
        - Erase an old message first
        ..................................................................*/
        if (display >= REDRAW_MESSAGE) {
            sprintf(txt, "%d ", MPlayerUnitCount);
            Fancy_Text_Print(txt,
                             Dimensions[BUTTON_COUNT].x + Dimensions[BUTTON_COUNT].w + 3 * Factor,
                             Dimensions[BUTTON_COUNT].y,
                             CC_GREEN,
                             BLACK,
                             TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            if (BuildLevel <= MPLAYER_BUILD_LEVEL_MAX) {
                sprintf(txt, "%d ", BuildLevel);
            } else {
                sprintf(txt, "**");
            }
            Fancy_Text_Print(txt,
                             Dimensions[BUTTON_LEVEL].x + Dimensions[BUTTON_LEVEL].w + 3 * Factor,
                             Dimensions[BUTTON_LEVEL].y,
                             CC_GREEN,
                             BLACK,
                             TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
            sprintf(txt, "%d ", MPlayerCredits);
            Fancy_Text_Print(txt,
                             Dimensions[BUTTON_CREDITS].x + Dimensions[BUTTON_CREDITS].w + 3 * Factor,
                             Dimensions[BUTTON_CREDITS].y,
                             CC_GREEN,
                             BLACK,
                             TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            sprintf(txt, "%dx ", Get_Control<GaugeClass>(BUTTON_TIBERIUMSCALE).Get_Value() + 1);
            Fancy_Text_Print(txt,
                             Dimensions[BUTTON_TIBERIUMSCALE].x + Dimensions[BUTTON_TIBERIUMSCALE].w + 3 * Factor,
                             Dimensions[BUTTON_TIBERIUMSCALE].y,
                             CC_GREEN,
                             BLACK,
                             TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
        }

        /*
        .......................... Redraw buttons ..........................
        */
        if (display >= REDRAW_BUTTONS) {
            CommandChain->Flag_List_To_Redraw();
        }

        Show_Mouse();
        display = REDRAW_NONE;
    }

    void Init_UI_State()
    {
        auto& option_list = Get_Control<CheckListClass>(BUTTON_OPTIONS);

        option_list.Set_Tabs(OptionTabs);
        option_list.Set_Read_Only(0);

        option_list.Add_Item(Text_String(TXT_BASES_ON));
        option_list.Add_Item("Tiberium Regrows"); // TODO: Locale file entry
        option_list.Add_Item(Text_String(TXT_CRATES_ON));
        //optionlist.Add_Item(Text_String(TXT_SHADOW_REGROWS)); // TODO: Implement for TD? (copied from RA)
        option_list.Add_Item(Text_String(TXT_CAPTURE_THE_FLAG));

        option_list.Check_Item(0, MPlayerBases);
        option_list.Check_Item(1, MPlayerTiberium);
        option_list.Check_Item(2, MPlayerGoodies);
        //optionlist.Check_Item(3, Special.IsShadowGrow);
        option_list.Check_Item(3, Special.IsCaptureTheFlag);

        auto& levelgauge = Get_Control<GaugeClass>(BUTTON_LEVEL);
        levelgauge.Set_Maximum(MPLAYER_BUILD_LEVEL_MAX - 1);
        levelgauge.Set_Value(BuildLevel - 1);

        auto& countgauge = Get_Control<GaugeClass>(BUTTON_COUNT);
        countgauge.Set_Maximum(MPlayerCountMax[MPlayerBases] - MPlayerCountMin[MPlayerBases]);
        countgauge.Set_Value(MPlayerUnitCount - MPlayerCountMin[MPlayerBases]);

        auto& creditsgauge = Get_Control<GaugeClass>(BUTTON_CREDITS);
        creditsgauge.Set_Maximum(Rule.Get_Rule_Value<int>(GAME_MULTIPLAYER_SECTION, START_CREDITS_MAX_RULE));
        creditsgauge.Set_Value(MPlayerCredits);

        auto& tiberiumscalegauge = Get_Control<GaugeClass>(BUTTON_TIBERIUMSCALE);
        tiberiumscalegauge.Set_Maximum(4);
        tiberiumscalegauge.Set_Value(MPlayerTiberium < 2 ? 0 : MPlayerTiberium - 1);

        auto& scenariolist = Get_Control<ListClass>(BUTTON_SCENARIOLIST);

        for (auto i = 0; i < MPlayerScenarios.Count(); i++) {
            scenariolist.Add_Item(strupr(MPlayerScenarios[i]));
        }
        ScenarioIdx = 0; // 1st scenario is selected

        // select the last scenario chosen by the player (if present)
        for (auto i = 0; i < MPlayerFilenum.Count(); i++) {
            if (MPlayerFilenum[i] == MPlayerScenarioNumber) {
                ScenarioIdx = i;
                scenariolist.Set_Selected_Index(i);
                break;
            }
        }
    }

    void Init_Data()
    {
        MPlayerColorIdx = MPlayerPrefColor; // init my preferred color

        strcpy(Text[BUTTON_NAME].get(), MPlayerName);       // set my name

        auto& name = Get_Control<EditClass>(BUTTON_NAME);

        name.Set_Text(Text[BUTTON_NAME].get(), MPLAYER_NAME_MAX);
        name.Set_Color(MPlayerTColors[MPlayerColorIdx]);

        auto& house = Get_Control<DropListClass>(BUTTON_HOUSE);

        // TODO: Add mystery option ? (random house selection)
        house.Add_Item(Text_String(TXT_G_D_I));
        house.Add_Item(Text_String(TXT_N_O_D));
        // TODO: Add dinosaur support (no bases ONLY)
        house.Set_Selected_Index(MPlayerHouse - HOUSE_GOOD);
        house.Set_Read_Only(true);

        if (MPlayerGhosts > 5) {
            MPlayerGhosts = 5;
        }
        MPlayerGhosts = max(MPlayerGhosts, 1);

        for (auto control = BUTTON_AI_DIFF_1; control <= BUTTON_AI_DIFF_5; ++control) {
            auto& dropdown = Get_Control<DropListClass>(control);

            dropdown.Add_Item("Disabled"); // TODO: Locale file entries
            dropdown.Add_Item("Easy");
            dropdown.Add_Item("Normal");
            dropdown.Add_Item("Hard");
            dropdown.Set_Selected_Index(control - BUTTON_AI_DIFF_1 < MPlayerGhosts ? 2 : 0);
            dropdown.Set_Read_Only(true);
        }

        for (auto control = BUTTON_AI_HOUSE_1; control <= BUTTON_AI_HOUSE_5; ++control) {
            auto& dropdown = Get_Control<DropListClass>(control);

            dropdown.Add_Item("None"); // TODO: Locale file entries
            dropdown.Add_Item("?");
            dropdown.Add_Item(Text_String(TXT_G_D_I));
            dropdown.Add_Item(Text_String(TXT_N_O_D));
            // TODO: Add dinosaur support (no bases ONLY)
            dropdown.Set_Selected_Index(control - BUTTON_AI_HOUSE_1 < MPlayerGhosts ? 1 : 0);
            dropdown.Set_Read_Only(true);
        }

        // GB 2022 set defaults for skirmish also:
        BuildLevel = 7;

        // init credits & credit buffer
        MPlayerCredits = Rule.Get_Rule_Value<int>(GAME_MULTIPLAYER_SECTION, START_CREDITS_DEFAULT_RULE);
        MPlayerGhosts = 1;
        MPlayerUnitCount = (MPlayerCountMax[MPlayerBases] + MPlayerCountMin[MPlayerBases]) / 2;
        MPlayerTiberium = 1;

        Special.IsTGrowth = MPlayerTiberium;
        Special.IsTSpread = MPlayerTiberium;

        for (auto& player_house : MPlayerHouses) {
            player_house = HOUSE_NONE;
        }

        for (auto& player_difficulty : MPlayerDifficulty) {
            player_difficulty = DIFF_NONE;
        }
    }

    void Init_Dialog_Buttons()
    {
        Controls[BUTTON_OK] = std::unique_ptr<GadgetClass>(
            new TextButtonClass(
                BUTTON_OK,
                TXT_OK,
                TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                Dimensions[BUTTON_OK].x,
                Dimensions[BUTTON_OK].y,
                Dimensions[BUTTON_OK].w,
                Dimensions[BUTTON_OK].h
            )
        );
        Get_Control<TextButtonClass>(BUTTON_OK).Add_Tail(*CommandChain);

        Controls[BUTTON_CANCEL] = std::unique_ptr<GadgetClass>(
            new TextButtonClass(
                BUTTON_CANCEL,
                TXT_CANCEL,
                TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                Dimensions[BUTTON_CANCEL].x,
                Dimensions[BUTTON_CANCEL].y,
                Dimensions[BUTTON_CANCEL].w,
                Dimensions[BUTTON_CANCEL].h
            )
        );
        Get_Control<TextButtonClass>(BUTTON_CANCEL).Add_Tail(*CommandChain);
    }

    void Init_Difficulty_Slider()
    {
        Controls[BUTTON_DIFFICULTY] = std::unique_ptr<GadgetClass>(
            new SliderClass(
                BUTTON_DIFFICULTY,
                Dimensions[BUTTON_NAME].x,
                Dimensions[BUTTON_OK].y - (8 * Factor) - MarginWidth,
                Width - (Dimensions[BUTTON_NAME].x - X) * 2,
                8 * Factor,
                true
            )
        );

        auto& difficulty = Get_Control<SliderClass>(BUTTON_DIFFICULTY);

        difficulty.Add_Tail(*CommandChain);

        if (Rule.IsFineDifficulty) {
            difficulty.Set_Maximum(5);
            difficulty.Set_Value(2);
        } else {
            difficulty.Set_Maximum(3);
            difficulty.Set_Value(1);
        }
    }

    void Init_Bottom_Row()
    {
        Controls[BUTTON_COUNT] = std::unique_ptr<GadgetClass>(
            new GaugeClass(BUTTON_COUNT, Dimensions[BUTTON_COUNT].x, Dimensions[BUTTON_COUNT].y, Dimensions[BUTTON_COUNT].w, Dimensions[BUTTON_COUNT].h)
        );
        Get_Control<GaugeClass>(BUTTON_COUNT).Add_Tail(*CommandChain);

        Controls[BUTTON_LEVEL] = std::unique_ptr<GadgetClass>(
            new GaugeClass(BUTTON_LEVEL, Dimensions[BUTTON_LEVEL].x, Dimensions[BUTTON_LEVEL].y, Dimensions[BUTTON_LEVEL].w, Dimensions[BUTTON_LEVEL].h)
        );
        Get_Control<GaugeClass>(BUTTON_LEVEL).Add_Tail(*CommandChain);

        Controls[BUTTON_CREDITS] = std::unique_ptr<GadgetClass>(
            new GaugeClass(BUTTON_CREDITS, Dimensions[BUTTON_CREDITS].x, Dimensions[BUTTON_CREDITS].y, Dimensions[BUTTON_CREDITS].w, Dimensions[BUTTON_CREDITS].h)
        );
        Get_Control<GaugeClass>(BUTTON_CREDITS).Add_Tail(*CommandChain);

        Controls[BUTTON_TIBERIUMSCALE] = std::unique_ptr<GadgetClass>(
            new GaugeClass(
                BUTTON_TIBERIUMSCALE,
                Dimensions[BUTTON_TIBERIUMSCALE].x,
                Dimensions[BUTTON_TIBERIUMSCALE].y,
                Dimensions[BUTTON_TIBERIUMSCALE].w,
                Dimensions[BUTTON_TIBERIUMSCALE].h
            )
        );
        Get_Control<GaugeClass>(BUTTON_TIBERIUMSCALE).Add_Tail(*CommandChain);

        Controls[BUTTON_OPTIONS] = std::unique_ptr<GadgetClass>(
            new CheckListClass(
                BUTTON_OPTIONS,
                Dimensions[BUTTON_OPTIONS].x,
                Dimensions[BUTTON_OPTIONS].y,
                Dimensions[BUTTON_OPTIONS].w,
                Dimensions[BUTTON_OPTIONS].h,
                TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                UpButtonShape,
                DownButtonShape
            )
        );
        Get_Control<GaugeClass>(BUTTON_OPTIONS).Add_Tail(*CommandChain);
    }

    void Init_Middle_Row()
    {
        for (auto control = BUTTON_AI_DIFF_1; control <= BUTTON_AI_DIFF_5; ++control) {
            const auto house_control = static_cast<ButtonType>(control + 5);

            Text[control] = std::make_unique<char[]>(DropdownTextLength);
            Text[house_control] = std::make_unique<char[]>(DropdownTextLength);

            Controls[control] = std::unique_ptr<GadgetClass>(
                new DropListClass(
                    control,
                    Text[control].get(),
                    DropdownTextLength,
                    TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                    Dimensions[control].x,
                    Dimensions[control].y,
                    Dimensions[control].w,
                    Dimensions[control].h,
                    UpButtonShape,
                    DownButtonShape
                )
            );
            Get_Control<DropListClass>(control).Add_Tail(*CommandChain);

            Controls[house_control] = std::unique_ptr<GadgetClass>(
                new DropListClass(
                    house_control,
                    Text[house_control].get(),
                    DropdownTextLength,
                    TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                    Dimensions[house_control].x,
                    Dimensions[house_control].y,
                    Dimensions[house_control].w,
                    Dimensions[house_control].h,
                    UpButtonShape,
                    DownButtonShape
                )
            );
            Get_Control<DropListClass>(house_control).Add_Tail(*CommandChain);
        }

        Controls[BUTTON_SCENARIOLIST] = std::unique_ptr<GadgetClass>(
            new ListClass(
                BUTTON_SCENARIOLIST,
                Dimensions[BUTTON_SCENARIOLIST].x,
                Dimensions[BUTTON_SCENARIOLIST].y,
                Dimensions[BUTTON_SCENARIOLIST].w,
                Dimensions[BUTTON_SCENARIOLIST].h,
                TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                UpButtonShape,
                DownButtonShape
            )
        );
        Get_Control<ListClass>(BUTTON_SCENARIOLIST).Add_Tail(*CommandChain);
    }

    void Init_Top_Row()
    {
        Text[BUTTON_NAME] = std::make_unique<char[]>(MPLAYER_NAME_MAX);
        Controls[BUTTON_NAME] = std::unique_ptr<GadgetClass>(
            new EditClass(
                BUTTON_NAME,
                Text[BUTTON_NAME].get(),
                MPLAYER_NAME_MAX,
                TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                Dimensions[BUTTON_NAME].x,
                Dimensions[BUTTON_NAME].y,
                Dimensions[BUTTON_NAME].w,
                Dimensions[BUTTON_NAME].h,
                EditClass::ALPHANUMERIC
            )
        );
        CommandChain = Controls[BUTTON_NAME].get();

        Text[BUTTON_HOUSE] = std::make_unique<char[]>(DropdownTextLength);
        Controls[BUTTON_HOUSE] = std::unique_ptr<GadgetClass>(
            new DropListClass(BUTTON_HOUSE,
                Text[BUTTON_HOUSE].get(),
                DropdownTextLength,
                TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                Dimensions[BUTTON_HOUSE].x,
                Dimensions[BUTTON_HOUSE].y,
                Dimensions[BUTTON_HOUSE].w,
                Dimensions[BUTTON_HOUSE].h,
                UpButtonShape,
                DownButtonShape
            )
        );
        Get_Control<DropListClass>(BUTTON_HOUSE).Add_Tail(*CommandChain);
    }

    void Init_Controls()
    {
        Text.clear();
        Controls.clear();
        CommandChain = nullptr;

        Init_Top_Row();
        Init_Middle_Row();
        Init_Bottom_Row();
        Init_Difficulty_Slider();
        Init_Dialog_Buttons();
    }

    void Init_Shapes()
    {
        if (InMainLoop || Factor == 1) {
            UpButtonShape = UpButtonShape == nullptr ? Hires_Retrieve("BTN-UP.SHP") : UpButtonShape;
            DownButtonShape = DownButtonShape == nullptr ? Hires_Retrieve("BTN-DN.SHP") : DownButtonShape;
        } else {
            UpButtonShape = UpButtonShape == nullptr ? Hires_Retrieve("BTN-UP2.SHP") : UpButtonShape;
            DownButtonShape = DownButtonShape == nullptr ? Hires_Retrieve("BTN-DN2.SHP") : DownButtonShape;
        }
    }

    void Init_Dimensions(const int screen_width, const int screen_height)
    {
        /*........................................................................
        Dialog & button dimensions
        ........................................................................*/
        Width = 300 * Factor;
        Height = 195 * Factor;
        X = (screen_width - Width) / 2;
        Y = (screen_height - Height) / 2;
        Center = X + (Width / 2);

        TextHeight = 6 * Factor + 1; // ht of 6-pt text
        MarginWidth = 10 * Factor;
        MarginHeight = 4 * Factor;

        Dimensions.clear();

        Dimensions[BUTTON_OK].w = 45 * Factor;
        Dimensions[BUTTON_OK].h = 9 * Factor;
        Dimensions[BUTTON_OK].x = X + (Width / 6) - (Dimensions[BUTTON_OK].w / 2);
        Dimensions[BUTTON_OK].y = Y + Height - Dimensions[BUTTON_OK].h - MarginWidth - Factor * 6;

        Dimensions[BUTTON_CANCEL].w = 45 * Factor;
        Dimensions[BUTTON_CANCEL].h = 9 * Factor;
        Dimensions[BUTTON_CANCEL].x = X + Width - (Width / 6) - (Dimensions[BUTTON_CANCEL].w / 2);
        Dimensions[BUTTON_CANCEL].y = Y + Height - Dimensions[BUTTON_CANCEL].h - MarginWidth - Factor * 6;

        Dimensions[BUTTON_NAME].w = 70 * Factor;
        Dimensions[BUTTON_NAME].h = 9 * Factor;
        Dimensions[BUTTON_NAME].x = X + 5 + (Width / 4) - (Dimensions[BUTTON_NAME].w / 2);
        Dimensions[BUTTON_NAME].y = Y + MarginHeight + TextHeight + 1 * Factor;

        Dimensions[BUTTON_HOUSE].w = 60 * Factor;
        Dimensions[BUTTON_HOUSE].h = (3 * 5 * Factor);
        Dimensions[BUTTON_HOUSE].x = Center - (Dimensions[BUTTON_HOUSE].w / 2);
        Dimensions[BUTTON_HOUSE].y = Dimensions[BUTTON_NAME].y;

        Dimensions[BUTTON_COLOR_1].w = 10 * Factor;
        Dimensions[BUTTON_COLOR_1].h = 9 * Factor;
        Dimensions[BUTTON_COLOR_1].y = Dimensions[BUTTON_NAME].y;
        Dimensions[BUTTON_COLOR_1].x = X + ((Width / 4) * 3) - (Dimensions[BUTTON_COLOR_1].w * 3);

        for (auto control = BUTTON_COLOR_2; control <= BUTTON_COLOR_6; ++control) {
            Dimensions[control] = Dimensions[BUTTON_COLOR_1];
            Dimensions[control].x = Dimensions[control].x + (Dimensions[control].w * (control - BUTTON_COLOR_1));
        }

        Dimensions[BUTTON_PLAYERLIST].w = 118 * Factor;
        Dimensions[BUTTON_PLAYERLIST].x = X + MarginWidth + MarginWidth + 5 * Factor;

        Dimensions[BUTTON_SCENARIOLIST].w = 140 * Factor;
        Dimensions[BUTTON_SCENARIOLIST].h = 30 * Factor;
        Dimensions[BUTTON_SCENARIOLIST].x =
            (Dimensions[BUTTON_CANCEL].x + static_cast<int>(nearbyint(Dimensions[BUTTON_CANCEL].w * 0.8)))
            - Dimensions[BUTTON_SCENARIOLIST].w + (20 * Factor);
        Dimensions[BUTTON_SCENARIOLIST].y = Dimensions[BUTTON_COLOR_1].y + TextHeight + 5 * Factor + TextHeight;

        Dimensions[BUTTON_SCENARIOLIST].h *= 2;

        // right column of AI house controls
        Dimensions[BUTTON_AI_HOUSE_1].w = static_cast<int>(nearbyint(Dimensions[BUTTON_HOUSE].w / 1.75));
        Dimensions[BUTTON_AI_HOUSE_1].h = (6 * 5 * Factor);
        Dimensions[BUTTON_AI_HOUSE_1].x = Dimensions[BUTTON_SCENARIOLIST].x
            - Dimensions[BUTTON_AI_HOUSE_1].w - (10 * Factor);
        Dimensions[BUTTON_AI_HOUSE_1].y = Dimensions[BUTTON_SCENARIOLIST].y + (5 * Factor);

        for (auto control = BUTTON_AI_HOUSE_2; control <= BUTTON_AI_HOUSE_5; ++control) {
            const auto previous_control = static_cast<ButtonType>(control - 1);

            Dimensions[control] = Dimensions[BUTTON_AI_HOUSE_1];
            Dimensions[control].y = Dimensions[previous_control].y + 10 * Factor;
        }

        // left column of AI house controls
        Dimensions[BUTTON_AI_DIFF_1].w = static_cast<int>(nearbyint(Dimensions[BUTTON_AI_HOUSE_1].w * 1.5));
        Dimensions[BUTTON_AI_DIFF_1].h = Dimensions[BUTTON_AI_HOUSE_1].h;
        Dimensions[BUTTON_AI_DIFF_1].x = Dimensions[BUTTON_AI_HOUSE_1].x
            - static_cast<int>(nearbyint(Dimensions[BUTTON_AI_HOUSE_1].w * 1.5)) - (10 * Factor);
        Dimensions[BUTTON_AI_DIFF_1].y = Dimensions[BUTTON_AI_HOUSE_1].y;

        for (auto control = BUTTON_AI_DIFF_2; control <= BUTTON_AI_DIFF_5; ++control) {
            const auto previous_control = static_cast<ButtonType>(control - 1);

            Dimensions[control] = Dimensions[BUTTON_AI_DIFF_1];
            Dimensions[control].y = Dimensions[previous_control].y + 10 * Factor;
        }

        Dimensions[BUTTON_OPTIONS].w = static_cast<int>(nearbyint(Dimensions[BUTTON_SCENARIOLIST].w * 0.8));
        Dimensions[BUTTON_OPTIONS].h = (5 * 6 * Factor) + 5 * Factor;
        Dimensions[BUTTON_OPTIONS].x = (Dimensions[BUTTON_SCENARIOLIST].x + Dimensions[BUTTON_SCENARIOLIST].w)
            - Dimensions[BUTTON_OPTIONS].w;
        Dimensions[BUTTON_OPTIONS].y = Dimensions[BUTTON_SCENARIOLIST].y + Dimensions[BUTTON_SCENARIOLIST].h
            + MarginWidth - 2 * Factor;

        Dimensions[BUTTON_COUNT].w = 25 * Factor;
        Dimensions[BUTTON_COUNT].h = 7 * Factor;
        Dimensions[BUTTON_COUNT].y = Dimensions[BUTTON_OPTIONS].y;
        Dimensions[BUTTON_COUNT].x = Dimensions[BUTTON_PLAYERLIST].x + (Dimensions[BUTTON_PLAYERLIST].w / 2)
            + 20 * Factor;

        Dimensions[BUTTON_LEVEL].w = 25 * Factor;
        Dimensions[BUTTON_LEVEL].h = 7 * Factor;
        Dimensions[BUTTON_LEVEL].y = Dimensions[BUTTON_COUNT].y + Dimensions[BUTTON_COUNT].h;
        Dimensions[BUTTON_LEVEL].x = Dimensions[BUTTON_PLAYERLIST].x + (Dimensions[BUTTON_PLAYERLIST].w / 2)
            + 20 * Factor;

        Dimensions[BUTTON_CREDITS].w = 25 * Factor;
        Dimensions[BUTTON_CREDITS].h = 7 * Factor;
        Dimensions[BUTTON_CREDITS].x = Dimensions[BUTTON_PLAYERLIST].x + (Dimensions[BUTTON_PLAYERLIST].w / 2)
            + 20 * Factor;
        Dimensions[BUTTON_CREDITS].y = Dimensions[BUTTON_LEVEL].y + Dimensions[BUTTON_LEVEL].h;

        Dimensions[BUTTON_TIBERIUMSCALE].w = 25 * Factor;
        Dimensions[BUTTON_TIBERIUMSCALE].h = 7 * Factor;
        Dimensions[BUTTON_TIBERIUMSCALE].x = Dimensions[BUTTON_PLAYERLIST].x + (Dimensions[BUTTON_PLAYERLIST].w / 2)
            + 20 * Factor;
        Dimensions[BUTTON_TIBERIUMSCALE].y = Dimensions[BUTTON_CREDITS].y + Dimensions[BUTTON_CREDITS].h;
    }

public:
    SkirmishScenarioDialog(const int factor)
    {
        Factor = factor;
        UpButtonShape = nullptr;
        DownButtonShape = nullptr;
        CommandChain = nullptr;
    }

    ~SkirmishScenarioDialog()
    {
        if (!Controls.contains(BUTTON_SCENARIOLIST) || Controls[BUTTON_SCENARIOLIST].get() == nullptr) {
            return;
        }

        auto& scenario_list = Get_Control<ListClass>(BUTTON_SCENARIOLIST);

        while (scenario_list.Count()) {
            // free dynamically allocated strings
            scenario_list.Remove_Item(scenario_list.Get_Item(0));
        }
    }

    void Init(const int screen_width, const int screen_height)
    {
        Init_Dimensions(screen_width, screen_height);
        Init_Shapes();
        Init_Controls();
        Init_Data();
        Init_UI_State();
    }

    bool Present()
    {
        auto display = REDRAW_ALL; // redraw level

        while (true) {
            /*
            ** If we have just received input focus again after running in the background then
            ** we need to redraw.
            */
            if (AllSurfaces.SurfacesRestored) {
                AllSurfaces.SurfacesRestored = false;
                display = REDRAW_ALL;
            }

            /*
            ........................ Invoke game callback .........................
            */
            Call_Back();

            /*
            ...................... Refresh display if needed ......................
            */
            Render(display);

            /*
            ........................... Get user input ............................
            */
            auto& house_dropdown = Get_Control<DropListClass>(BUTTON_HOUSE);
            const bool droplist_is_dropped = house_dropdown.IsDropped;
            std::vector<ButtonType> ai_diffs_collapsed;
            std::vector<ButtonType> ai_houses_collapsed;

            for (auto control = BUTTON_AI_DIFF_1; control <= BUTTON_AI_DIFF_5; ++control) {
                if (Get_Control<DropListClass>(control).IsDropped) {
                    ai_diffs_collapsed.emplace_back(control);
                }

                const auto house_btn = static_cast<ButtonType>(control + 5);

                if (Get_Control<DropListClass>(house_btn).IsDropped) {
                    ai_houses_collapsed.emplace_back(house_btn);
                }
            }

            auto input = CommandChain->Input();

            /*
            ** Redraw everything if the player house droplist collapsed
            */
            if (droplist_is_dropped && !house_dropdown.IsDropped) {
                display = REDRAW_BACKGROUND;
            }

            /*
            ** Redraw everything if an AI house droplist collapsed
            */
            for (const auto& control: ai_diffs_collapsed) {
                if (!Get_Control<DropListClass>(control).IsDropped) {
                    display = REDRAW_BACKGROUND;
                }
            }

            for (const auto& control: ai_houses_collapsed) {
                if (!Get_Control<DropListClass>(control).IsDropped) {
                    display = REDRAW_BACKGROUND;
                }
            }

            const auto collapse_visible_dropdowns = [&]() {
                if (house_dropdown.IsDropped) {
                    house_dropdown.Collapse();
                    display = REDRAW_BACKGROUND;
                }

                for (auto control = BUTTON_AI_DIFF_1; control <= BUTTON_AI_DIFF_5; ++control) {
                    const auto house_btn = static_cast<ButtonType>(control + 5);
                    auto& diff_dropdown = Get_Control<DropListClass>(control);
                    auto& house_dropdown = Get_Control<DropListClass>(house_btn);

                    if (diff_dropdown.IsDropped) {
                        diff_dropdown.Collapse();
                        display = REDRAW_BACKGROUND;
                    }

                    if (house_dropdown.IsDropped) {
                        house_dropdown.Collapse();
                        display = REDRAW_BACKGROUND;
                    }
                }
            };

            if (input & KN_BUTTON) {
                // user is interacting with a button, so hide dropdown lists
                collapse_visible_dropdowns();
            }

            // for any visible AI house dropdown, if mouse input is received outside it's bounds, hide it
            for (auto control = BUTTON_AI_DIFF_1; control <= BUTTON_AI_DIFF_5; ++control) {
                const auto house_btn = static_cast<ButtonType>(control + 5);
                auto& diff_dropdown = Get_Control<DropListClass>(control);
                auto& house_dropdown = Get_Control<DropListClass>(house_btn);

                if (diff_dropdown.IsDropped) {
                    if ((Keyboard->MouseQX < diff_dropdown.X || Keyboard->MouseQX > diff_dropdown.X + diff_dropdown.Width)
                    && (Keyboard->MouseQY < diff_dropdown.Y || Keyboard->MouseQY > diff_dropdown.Y + diff_dropdown.Height)) {
                        diff_dropdown.Collapse();
                        display = REDRAW_BACKGROUND;
                    }
                }

                if (house_dropdown.IsDropped) {
                    if ((Keyboard->MouseQX < house_dropdown.X || Keyboard->MouseQX > house_dropdown.X + house_dropdown.Width)
                    && (Keyboard->MouseQY < house_dropdown.Y || Keyboard->MouseQY > house_dropdown.Y + house_dropdown.Height)) {
                        house_dropdown.Collapse();
                        display = REDRAW_BACKGROUND;
                    }
                }
            }

            /*
            ---------------------------- Process input ----------------------------
            */
            switch (input) {
                /*------------------------------------------------------------------
                User clicks on a color button
                ------------------------------------------------------------------*/
                case KN_LMOUSE:
                    if (Keyboard->MouseQX > Dimensions[BUTTON_COLOR_1].x && Keyboard->MouseQX < (Dimensions[BUTTON_COLOR_6].x + Dimensions[BUTTON_COLOR_6].w)
                        && Keyboard->MouseQY > Dimensions[BUTTON_COLOR_1].y && Keyboard->MouseQY < (Dimensions[BUTTON_COLOR_6].y + Dimensions[BUTTON_COLOR_6].h)) {
                        MPlayerPrefColor = (Keyboard->MouseQX - Dimensions[BUTTON_COLOR_1].x) / Dimensions[BUTTON_COLOR_1].w;
                        MPlayerColorIdx = MPlayerPrefColor;
                        display = REDRAW_COLORS;

                        auto& name_edt = Get_Control<EditClass>(BUTTON_NAME);

                        name_edt.Set_Color(MPlayerTColors[MPlayerColorIdx]);
                        name_edt.Flag_To_Redraw();
                        strcpy(MPlayerName, Text[BUTTON_NAME].get());

                        collapse_visible_dropdowns();
                    }
                    break;

                    /*------------------------------------------------------------------
                    User edits the name field; retransmit new game options
                    ------------------------------------------------------------------*/
                case (BUTTON_NAME | KN_BUTTON): {
                    strcpy(MPlayerName, Text[BUTTON_NAME].get());
                    collapse_visible_dropdowns();
                    break;
                }

                    /*------------------------------------------------------------------
                    House Buttons: set the player's desired House
                    ------------------------------------------------------------------*/
                case (BUTTON_HOUSE | KN_BUTTON): {
                    MPlayerHouse = HousesType(house_dropdown.Current_Index() + HOUSE_GOOD);
                    strcpy(MPlayerName, Text[BUTTON_NAME].get());

                    collapse_visible_dropdowns();

                    display = REDRAW_BACKGROUND;
                    break;
                }

                    /*------------------------------------------------------------------
                    New Scenario selected.
                    ------------------------------------------------------------------*/
                case (BUTTON_SCENARIOLIST | KN_BUTTON): {
                    auto& scenariolist = Get_Control<ListClass>(BUTTON_SCENARIOLIST);
                    if (scenariolist.Current_Index() != ScenarioIdx) {
                        ScenarioIdx = scenariolist.Current_Index();

                        // store the scenario number rather than current scenario list index
                        // (index will change if maps are added/removed by player)
                        MPlayerScenarioNumber = MPlayerFilenum[ScenarioIdx];

                        strcpy(MPlayerName, Text[BUTTON_NAME].get());
                    }
                    break;
                }

                    /*------------------------------------------------------------------
                    AI player difficulty dropdown selection changed.
                    ------------------------------------------------------------------*/
                case (BUTTON_AI_DIFF_1 | KN_BUTTON):
                case (BUTTON_AI_DIFF_2 | KN_BUTTON):
                case (BUTTON_AI_DIFF_3 | KN_BUTTON):
                case (BUTTON_AI_DIFF_4 | KN_BUTTON):
                case (BUTTON_AI_DIFF_5 | KN_BUTTON): {
                    for (auto control = BUTTON_AI_DIFF_1; control <= BUTTON_AI_DIFF_5; ++control) {
                        if (input != (control | KN_BUTTON)) {
                            continue;
                        }

                        const auto house_btn = static_cast<ButtonType>(control + 5);
                        auto& diff_dropdown = Get_Control<DropListClass>(control);
                        auto& house_dropdown = Get_Control<DropListClass>(house_btn);

                        // nothing changed, ignore input
                        if (!diff_dropdown.List.Index_Changed()) {
                            break;
                        }

                        const auto diff_was_disabled = diff_dropdown.Current_Index() == 0;
                        const auto diff_was_activated = diff_dropdown.List.Get_Previous_Index() == 0;
                        const auto house_is_disabled = house_dropdown.Current_Index() == 0;

                        if (diff_was_disabled) {
                            // reset house to match disabled AI diff
                            house_dropdown.Set_Selected_Index(0);
                        } else if (diff_was_activated && house_is_disabled) {
                            // select a default house of '?' since none is selected
                            house_dropdown.Set_Selected_Index(1);
                        }

                        diff_dropdown.Collapse();
                        house_dropdown.Collapse();
                        display = REDRAW_BACKGROUND;
                        break;
                    }

                    break;
                }

                    /*------------------------------------------------------------------
                    AI player house dropdown selection changed.
                    ------------------------------------------------------------------*/
                case (BUTTON_AI_HOUSE_1 | KN_BUTTON):
                case (BUTTON_AI_HOUSE_2 | KN_BUTTON):
                case (BUTTON_AI_HOUSE_3 | KN_BUTTON):
                case (BUTTON_AI_HOUSE_4 | KN_BUTTON):
                case (BUTTON_AI_HOUSE_5 | KN_BUTTON): {
                    for (auto control = BUTTON_AI_HOUSE_1; control <= BUTTON_AI_HOUSE_5; ++control) {
                        if (input != (control | KN_BUTTON)) {
                            continue;
                        }

                        const auto diff_btn = static_cast<ButtonType>(control - 5);
                        auto& diff_dropdown = Get_Control<DropListClass>(diff_btn);
                        auto& house_dropdown = Get_Control<DropListClass>(control);

                        // nothing changed, ignore input
                        if (!house_dropdown.List.Index_Changed()) {
                            break;
                        }

                        const auto house_was_disabled = house_dropdown.Current_Index() == 0;
                        const auto house_was_activated = house_dropdown.List.Get_Previous_Index() == 0;
                        const auto diff_is_disabled = diff_dropdown.Current_Index() == 0;

                        if (house_was_disabled) {
                            // reset diff to match disabled AI house
                            diff_dropdown.Set_Selected_Index(0);
                        } else if (house_was_activated && diff_is_disabled) {
                            // select a default diff of 'Normal' since none is selected
                            diff_dropdown.Set_Selected_Index(2);
                        }

                        house_dropdown.Collapse();
                        diff_dropdown.Collapse();
                        display = REDRAW_BACKGROUND;
                    }

                    break;
                }

                /*------------------------------------------------------------------
                User adjusts max # units
                ------------------------------------------------------------------*/
                case (BUTTON_COUNT | KN_BUTTON): {
                    MPlayerUnitCount = Get_Control<GaugeClass>(BUTTON_COUNT).Get_Value()
                        + MPlayerCountMin[MPlayerBases];
                    if (display < REDRAW_MESSAGE) {
                        display = REDRAW_MESSAGE;
                    }
                    collapse_visible_dropdowns();
                    break;
                }

                /*------------------------------------------------------------------
                User adjusts build level
                ------------------------------------------------------------------*/
                case (BUTTON_LEVEL | KN_BUTTON): {
                    BuildLevel = Get_Control<GaugeClass>(BUTTON_LEVEL).Get_Value() + 1;
                    if (BuildLevel > MPLAYER_BUILD_LEVEL_MAX) // if it's pegged, max it out
                        BuildLevel = MPLAYER_BUILD_LEVEL_MAX;
                    if (display < REDRAW_MESSAGE) {
                        display = REDRAW_MESSAGE;
                    }
                    collapse_visible_dropdowns();
                    break;
                }

                /*------------------------------------------------------------------
                User edits the credits value; retransmit new game options
                ------------------------------------------------------------------*/
                case (BUTTON_CREDITS | KN_BUTTON): {
                    MPlayerCredits = Get_Control<GaugeClass>(BUTTON_CREDITS).Get_Value();

                    if (MPlayerCredits == 0) {
                        // clear lingering digits when player quickly slides to zero
                        display = REDRAW_ALL;
                    }

                    if (display < REDRAW_MESSAGE) {
                        display = REDRAW_MESSAGE;
                    }
                    collapse_visible_dropdowns();
                    break;
                }

                case (BUTTON_TIBERIUMSCALE | KN_BUTTON): {
                    MPlayerTiberium = Get_Control<GaugeClass>(BUTTON_TIBERIUMSCALE).Get_Value() + 1;

                    Get_Control<CheckListClass>(BUTTON_OPTIONS).Check_Item(1, MPlayerTiberium > 0);

                    Special.IsTGrowth = MPlayerTiberium;
                    Special.IsTSpread = MPlayerTiberium;

                    if (display < REDRAW_MESSAGE)
                        display = REDRAW_MESSAGE;

                    collapse_visible_dropdowns();
                    break;
                }

                /*------------------------------------------------------------------
                Toggle-able options:
                If 'Bases' gets toggled, we have to change the range of the
                UnitCount slider.
                Also, if Tiberium gets toggled, we have to set the flags
                in SpecialClass.
                ------------------------------------------------------------------*/
                case (BUTTON_OPTIONS | KN_BUTTON): {
                    auto& optionlist = Get_Control<CheckListClass>(BUTTON_OPTIONS);
                    auto countgauge = Get_Control<GaugeClass>(BUTTON_COUNT);
                    auto tiberiumscalegauge = Get_Control<GaugeClass>(BUTTON_TIBERIUMSCALE);

                    if (MPlayerBases != optionlist.Is_Checked(0)) {
                        MPlayerBases = optionlist.Is_Checked(0);
                        if (MPlayerBases) {
                            MPlayerUnitCount = Fixed_To_Cardinal(MPlayerCountMax[1] - MPlayerCountMin[1],
                                                                 Cardinal_To_Fixed(MPlayerCountMax[0] - MPlayerCountMin[0],
                                                                                   MPlayerUnitCount - MPlayerCountMin[0]))
                                               + MPlayerCountMin[1];
                        } else {
                            MPlayerUnitCount = Fixed_To_Cardinal(MPlayerCountMax[0] - MPlayerCountMin[0],
                                                                 Cardinal_To_Fixed(MPlayerCountMax[1] - MPlayerCountMin[1],
                                                                                   MPlayerUnitCount - MPlayerCountMin[1]))
                                               + MPlayerCountMin[0];
                        }
                        countgauge.Set_Maximum(MPlayerCountMax[MPlayerBases] - MPlayerCountMin[MPlayerBases]);
                        countgauge.Set_Value(MPlayerUnitCount - MPlayerCountMin[MPlayerBases]);
                    }
                    MPlayerTiberium = optionlist.Is_Checked(1) ? 1 : 0;

                    if (tiberiumscalegauge.Get_Value() + 1 > 1) {
                        MPlayerTiberium = tiberiumscalegauge.Get_Value() + 1;
                    }

                    tiberiumscalegauge.Set_Value(MPlayerTiberium < 2 ? 0 : MPlayerTiberium - 1);

                    Special.IsTGrowth = MPlayerTiberium;
                    Special.IsTSpread = MPlayerTiberium;

                    MPlayerGoodies = optionlist.Is_Checked(2);
                    Special.IsCaptureTheFlag = optionlist.Is_Checked(3);

                    if (display < REDRAW_MESSAGE)
                        display = REDRAW_MESSAGE;

                    collapse_visible_dropdowns();
                    break;
                }

                /*------------------------------------------------------------------
                OK: exit loop with true status
                ------------------------------------------------------------------*/
                case (BUTTON_OK | KN_BUTTON): {
                    // check if at least one AI player enabled
                    auto ai_players = false;

                    for (auto button = BUTTON_AI_DIFF_1; button <= BUTTON_AI_DIFF_5; ++button) {
                        if (Get_Control<DropListClass>(button).Current_Index() > 0) {
                            ai_players = true;
                            break;
                        }
                    }

                    if (ai_players) {
                        // at least one AI player enabled, allow user to proceed
                        return true;
                    }

                    // warn that no AI players are enabled
                    WWMessageBox().Process(TXT_ONLY_ONE, TXT_OOPS);
                    display = REDRAW_ALL;
                    break;
                }

                /*------------------------------------------------------------------
                CANCEL: send a SIGN_OFF, bail out with error code
                ------------------------------------------------------------------*/
                case (KN_ESC): {
                    if (Messages.Get_Edit_Buf() != NULL) {
                        Messages.Input(input);
                        if (display < REDRAW_MESSAGE)
                            display = REDRAW_MESSAGE;
                        break;
                    }
                }
                case (BUTTON_CANCEL | KN_BUTTON): return false;

                default: break;
            } /* end of input processing */

            Frame_Limiter();
        }
    }

    void Apply_Skirmish_Settings()
    {
        /*.....................................................................
        Set the number of players in this game, and my ID
        .....................................................................*/
        MPlayerCount = 1;
        MPlayerLocalID = Build_MPlayerID(MPlayerColorIdx, MPlayerHouse);

        /*.....................................................................
        Store every player's ID in the MPlayerID[] array.  This array will
        determine the order of event execution, so the ID's must be stored
        in the same order on all systems.
        .....................................................................*/
        MPlayerID[0] = MPlayerLocalID;
        strcpy(MPlayerName, Text[BUTTON_NAME].get());
        strcpy(MPlayerNames[0], MPlayerName);

        /*.....................................................................
        Get the scenario filename
        .....................................................................*/
        Scen.Scenario = MPlayerFilenum[ScenarioIdx];

        int diff = Get_Control<SliderClass>(BUTTON_DIFFICULTY).Get_Value() * (Rule.IsFineDifficulty ? 1 : 2);
        switch (diff) {
        case 0:
            Scen.CDifficulty = DIFF_HARD;
            Scen.Difficulty = DIFF_EASY;
            break;

        case 1:
            Scen.CDifficulty = DIFF_HARD;
            Scen.Difficulty = DIFF_NORMAL;
            break;

        case 2:
            Scen.CDifficulty = DIFF_NORMAL;
            Scen.Difficulty = DIFF_NORMAL;
            break;

        case 3:
            Scen.CDifficulty = DIFF_EASY;
            Scen.Difficulty = DIFF_NORMAL;
            break;

        case 4:
            Scen.CDifficulty = DIFF_EASY;
            Scen.Difficulty = DIFF_HARD;
            break;
        }

        // set AI player variables from difficulty/house dropdowns
        MPlayerGhosts = 0;

        for (auto control = BUTTON_AI_DIFF_1; control <= BUTTON_AI_DIFF_5; ++control) {
            const auto house_btn = static_cast<ButtonType>(control + 5);
            auto& diff_dropdown = Get_Control<DropListClass>(control);
            auto& house_dropdown = Get_Control<DropListClass>(house_btn);

            // skip disabled slots
            if (diff_dropdown.Current_Index() == 0) {
                continue;
            }

            // populate 'slot' with correct difficulty and house overrides
            const auto selected_diff_idx = diff_dropdown.Current_Index();
            const auto selected_house_idx = house_dropdown.Current_Index();

            const auto selected_diff = selected_diff_idx == 0
                ? DIFF_NONE
                : static_cast<DiffType>(selected_diff_idx - 1);
            const auto selected_house = selected_house_idx < 2
                ? HOUSE_NONE
                : static_cast<HousesType>(selected_house_idx - 2);

            MPlayerHouses[MPlayerGhosts + 1] = selected_house;
            MPlayerDifficulty[MPlayerGhosts + 1] = selected_diff;

            MPlayerGhosts++;
        }
    }
};

/***********************************************************************************************
 * Com_Scenario_Dialog -- Serial game scenario selection dialog                                *
 *                                                                                             *
 *                                                                                             *
 *    ┌────────────────────────────────────────────────────────────┐                           *
 *    │                        Serial Game                         │                           *
 *    │                                                            │                           *
 *    │     Your Name: __________          House: [GDI] [NOD]      │                           *
 *    │       Credits: ______      Desired Color: [ ][ ][ ][ ]     │                           *
 *    │      Opponent: Name                                        │                           *
 *    │                                                            │                           *
 *    │                         Scenario                           │                           *
 *    │                  ┌──────────────────┬─┐                    │                           *
 *    │                  │ Hell's Kitchen   │ │                    │                           *
 *    │                  │ Heaven's Gate    ├─┤                    │                           *
 *    │                  │      ...         │ │                    │                           *
 *    │                  │                  ├─┤                    │                           *
 *    │                  │                  │ │                    │                           *
 *    │                  └──────────────────┴─┘                    │                           *
 *    │                 [  Bases   ] [ Crates     ]                │                           *
 *    │                 [ Tiberium ] [ AI Players ]                │                           *
 *    │                                                            │                           *
 *    │                      [OK]    [Cancel]                      │                           *
 *    │   ┌────────────────────────────────────────────────────┐   │                           *
 *    │   │                                                    │   │                           *
 *    │   │                                                    │   │                           *
 *    │   └────────────────────────────────────────────────────┘   │                           *
 *    │                       [Send Message]                       │                           *
 *    └────────────────────────────────────────────────────────────┘                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      true = success, false = cancel                                                         *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      MPlayerName & MPlayerGameName must contain this player's name.                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *=============================================================================================*/
#define TXT_HOST_INTERNET_GAME 4567 + 1
#define TXT_JOIN_INTERNET_GAME 4567 + 2
int Com_Scenario_Dialog(void)
{
    const auto factor = (SeenBuff.Get_Width() == 320) ? 1 : 2;
    const auto width = Try_Get_Resolution_Mode_Width().value_or(SeenBuff.Get_Width());
    const auto height = Try_Get_Resolution_Mode_Height().value_or(SeenBuff.Get_Height());

    auto dialog = SkirmishScenarioDialog(factor);
    dialog.Init(width, height);

    Load_Title_Screen(TitlePicture, &HidPage, Palette);
    Blit_Hid_Page_To_Seen_Buff();
    Set_Palette(Palette);

    while (Get_Mouse_State() > 0)
        Show_Mouse();

    const auto result = dialog.Present();

    if (result) {
        dialog.Apply_Skirmish_Settings();
    }

    /*------------------------------------------------------------------------
    Restore screen
    ------------------------------------------------------------------------*/
    Hide_Mouse();
    Load_Title_Screen(TitlePicture, &HidPage, Palette);
    Blit_Hid_Page_To_Seen_Buff();
    Show_Mouse();

    /*------------------------------------------------------------------------
    Save any changes made to our options
    ------------------------------------------------------------------------*/
    Write_MultiPlayer_Settings();

    return result;
} /* end of Com_Scenario_Dialog */
