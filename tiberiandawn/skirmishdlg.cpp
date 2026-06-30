#include "function.h"
#include "drop.h"
#include "framelimit.h"

struct ControlDimension
{
    int X = 0;
    int Y = 0;
    int W = 0;
    int H = 0;
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
        BUTTON_OPTIONS,
        BUTTON_SCENARIO_LIST,
        BUTTON_COUNT,
        BUTTON_LEVEL,
        BUTTON_CREDITS,
        BUTTON_TIBERIUM_SCALE,
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
        BUTTON_PLAYER_LIST // not used
    } ControlType;

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

    // display scale
    int Factor;

    // dimensions

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

    // controls

    std::map<ControlType, ControlDimension> Dimensions;
    std::map<ControlType, std::unique_ptr<char[]>> Text;
    std::map<ControlType, std::unique_ptr<GadgetClass>> Controls;
    GadgetClass* CommandChain;

    bool Is_Mouse_Over_Rectangle(const int start_x, const int start_y, const int end_x, const int end_y)
    {
        return Keyboard->MouseQX >= start_x
            && Keyboard->MouseQX <= end_x
            && Keyboard->MouseQY >= start_y
            && Keyboard->MouseQY <= end_y;
    }

    bool Is_Mouse_Outside_Control_Dimensions(const GadgetClass& control)
    {
        return (Keyboard->MouseQX < control.X || Keyboard->MouseQX > control.X + control.Width)
            && (Keyboard->MouseQY < control.Y || Keyboard->MouseQY > control.Y + control.Height);
    }

    template<class T>
    T& Get_Control(const ControlType type)
    {
        return *reinterpret_cast<T*>(Controls[type].get());
    }

    KeyNumType Get_Input(RedrawType& display)
    {
        auto& house_dropdown = Get_Control<DropListClass>(BUTTON_HOUSE);
        const bool droplist_is_dropped = house_dropdown.IsDropped;
        std::vector<ControlType> ai_diffs_collapsed;
        std::vector<ControlType> ai_houses_collapsed;

        for (auto control = BUTTON_AI_DIFF_1; control <= BUTTON_AI_DIFF_5; ++control) {
            if (Get_Control<DropListClass>(control).IsDropped) {
                ai_diffs_collapsed.emplace_back(control);
            }

            const auto house_btn = static_cast<ControlType>(control + 5);

            if (Get_Control<DropListClass>(house_btn).IsDropped) {
                ai_houses_collapsed.emplace_back(house_btn);
            }
        }

        const auto input = CommandChain->Input();

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

        if (input & KN_BUTTON) {
            // user is interacting with a button, so hide dropdown lists
            Collapse_Visible_Dropdowns(display);
        }

        // for any visible AI house dropdown, if mouse input is received outside it's bounds, hide it
        for (auto control = BUTTON_AI_DIFF_1; control <= BUTTON_AI_DIFF_5; ++control) {
            const auto house_btn = static_cast<ControlType>(control + 5);
            auto& diff_dropdown = Get_Control<DropListClass>(control);
            auto& ai_house_dropdown = Get_Control<DropListClass>(house_btn);

            if (diff_dropdown.IsDropped && Is_Mouse_Outside_Control_Dimensions(diff_dropdown)) {
                diff_dropdown.Collapse();
                display = REDRAW_BACKGROUND;
            }

            if (ai_house_dropdown.IsDropped && Is_Mouse_Outside_Control_Dimensions(ai_house_dropdown)) {
                ai_house_dropdown.Collapse();
                display = REDRAW_BACKGROUND;
            }
        }

        return input;
    }

    void Render(RedrawType& display)
    {
        if (!display) {
            return;
        }
        char txt[80];

        Hide_Mouse();
        /*
        .................. Redraw background & dialog box ...................
        */
        if (display >= REDRAW_BACKGROUND) {
            Dialog_Box(X, Y, Width, Height);

            // init font variables

            Fancy_Text_Print(
                TXT_NONE, 0, 0, TBLACK, TBLACK, TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW
            );

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
                             Dimensions[BUTTON_NAME].X + (Dimensions[BUTTON_NAME].W / 2),
                             Dimensions[BUTTON_NAME].Y - TextHeight - (1 * Factor),
                             CC_GREEN,
                             TBLACK,
                             TPF_RIGHT | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            Fancy_Text_Print(TXT_SIDE_COLON,
                             Dimensions[BUTTON_HOUSE].X + (Dimensions[BUTTON_HOUSE].W / 2),
                             Dimensions[BUTTON_HOUSE].Y - TextHeight - (1 * Factor),
                             CC_GREEN,
                             TBLACK,
                             TPF_RIGHT | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            Fancy_Text_Print(TXT_COLOR_COLON,
                             X + ((Width / 4) * 3),
                             Dimensions[BUTTON_COLOR_1].Y - TextHeight - (1 * Factor),
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
                             Dimensions[BUTTON_SCENARIO_LIST].X + (Dimensions[BUTTON_SCENARIO_LIST].W / 2),
                             Dimensions[BUTTON_SCENARIO_LIST].Y - TextHeight - (1 * Factor),
                             CC_GREEN,
                             TBLACK,
                             TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            Fancy_Text_Print(TXT_COUNT,
                             Dimensions[BUTTON_COUNT].X - 3 * Factor,
                             Dimensions[BUTTON_COUNT].Y,
                             CC_GREEN,
                             TBLACK,
                             TPF_NOSHADOW | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_RIGHT);

            Fancy_Text_Print(TXT_LEVEL,
                             Dimensions[BUTTON_LEVEL].X - 3 * Factor,
                             Dimensions[BUTTON_LEVEL].Y,
                             CC_GREEN,
                             TBLACK,
                             TPF_NOSHADOW | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_RIGHT);

            Fancy_Text_Print(TXT_START_CREDITS_COLON,
                             Dimensions[BUTTON_CREDITS].X - 3 * Factor,
                             Dimensions[BUTTON_CREDITS].Y,
                             CC_GREEN,
                             TBLACK,
                             TPF_RIGHT | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            Fancy_Text_Print("Tiberium Growth:", // TODO: Locale file entry
                             Dimensions[BUTTON_TIBERIUM_SCALE].X - 3 * Factor,
                             Dimensions[BUTTON_TIBERIUM_SCALE].Y,
                             CC_GREEN,
                             TBLACK,
                             TPF_RIGHT | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            // AI player setting headers
            Fancy_Text_Print("Player", // TODO: Locale file entry
                             (Dimensions[BUTTON_AI_HOUSE_1].X
                                - static_cast<int>(nearbyint(Dimensions[BUTTON_AI_HOUSE_1].W * 1.5)) - (10 * Factor))
                                + (static_cast<int>((Dimensions[BUTTON_AI_HOUSE_1].W * 1.5) / 1.25)),
                             Dimensions[BUTTON_AI_HOUSE_1].Y - TextHeight - (2 * Factor),
                             CC_GREEN,
                             TBLACK,
                             TPF_RIGHT | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            Fancy_Text_Print("Side", // TODO: Locale file entry
                             Dimensions[BUTTON_AI_HOUSE_1].X
                                + static_cast<int>(nearbyint(Dimensions[BUTTON_AI_HOUSE_1].W / 1.25)),
                             Dimensions[BUTTON_AI_HOUSE_1].Y - TextHeight - (2 * Factor),
                             CC_GREEN,
                             TBLACK,
                             TPF_RIGHT | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            // AI player difficulty and house checkbox labels
            const auto cur_ai_house_label_x = (Dimensions[BUTTON_AI_HOUSE_1].X
                - static_cast<int>(nearbyint(Dimensions[BUTTON_AI_HOUSE_1].W * 1.5))
                - (10 * Factor)) - 3 * Factor;

            for (auto control = BUTTON_AI_DIFF_1; control <= BUTTON_AI_DIFF_5; ++control) {
                // TODO: Locale file entry
                Fancy_Text_Print(std::format("AI {}:", control - BUTTON_HOUSE).c_str(),
                                 cur_ai_house_label_x,
                                 Dimensions[control].Y,
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

                LogicPage->Fill_Rect(Dimensions[control].X + 1 * Factor,
                                     Dimensions[control].Y + 1 * Factor,
                                     Dimensions[control].X + 1 * Factor + Dimensions[control].W - 2 * Factor,
                                     Dimensions[control].Y + 1 * Factor + Dimensions[control].H - 2 * Factor,
                                     MPlayerGColors[mplayer_idx]);

                Draw_Box(
                    Dimensions[control].X,
                    Dimensions[control].Y,
                    Dimensions[control].W,
                    Dimensions[control].H,
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
                             Dimensions[BUTTON_COUNT].X + Dimensions[BUTTON_COUNT].W + 3 * Factor,
                             Dimensions[BUTTON_COUNT].Y,
                             CC_GREEN,
                             BLACK,
                             TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            if (BuildLevel <= MPLAYER_BUILD_LEVEL_MAX) {
                sprintf(txt, "%d ", BuildLevel);
            } else {
                sprintf(txt, "**");
            }
            Fancy_Text_Print(txt,
                             Dimensions[BUTTON_LEVEL].X + Dimensions[BUTTON_LEVEL].W + 3 * Factor,
                             Dimensions[BUTTON_LEVEL].Y,
                             CC_GREEN,
                             BLACK,
                             TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
            sprintf(txt, "%d ", MPlayerCredits);
            Fancy_Text_Print(txt,
                             Dimensions[BUTTON_CREDITS].X + Dimensions[BUTTON_CREDITS].W + 3 * Factor,
                             Dimensions[BUTTON_CREDITS].Y,
                             CC_GREEN,
                             BLACK,
                             TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            sprintf(txt, "%dx ", Get_Control<GaugeClass>(BUTTON_TIBERIUM_SCALE).Get_Value() + 1);
            Fancy_Text_Print(txt,
                             Dimensions[BUTTON_TIBERIUM_SCALE].X + Dimensions[BUTTON_TIBERIUM_SCALE].W + 3 * Factor,
                             Dimensions[BUTTON_TIBERIUM_SCALE].Y,
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

    void Collapse_Visible_Dropdowns(RedrawType& display)
    {
        auto& house_dropdown = Get_Control<DropListClass>(BUTTON_HOUSE);

        if (house_dropdown.IsDropped) {
            house_dropdown.Collapse();
            display = REDRAW_BACKGROUND;
        }

        for (auto control = BUTTON_AI_DIFF_1; control <= BUTTON_AI_DIFF_5; ++control) {
            const auto house_btn = static_cast<ControlType>(control + 5);
            auto& ai_diff_dropdown = Get_Control<DropListClass>(control);
            auto& ai_house_dropdown = Get_Control<DropListClass>(house_btn);

            if (ai_diff_dropdown.IsDropped) {
                ai_diff_dropdown.Collapse();
                display = REDRAW_BACKGROUND;
            }

            if (ai_house_dropdown.IsDropped) {
                ai_house_dropdown.Collapse();
                display = REDRAW_BACKGROUND;
            }
        }
    }

    void Init_UI_State()
    {
        auto& option_list = Get_Control<CheckListClass>(BUTTON_OPTIONS);

        option_list.Set_Tabs(OptionTabs);
        option_list.Set_Read_Only(0);

        option_list.Add_Item(Text_String(TXT_BASES_ON));
        option_list.Add_Item("Tiberium Regrows"); // TODO: Locale file entry
        option_list.Add_Item(Text_String(TXT_CRATES_ON));
        //option_list.Add_Item(Text_String(TXT_SHADOW_REGROWS)); // TODO: Implement for TD? (copied from RA)
        option_list.Add_Item(Text_String(TXT_CAPTURE_THE_FLAG));

        option_list.Check_Item(0, MPlayerBases);
        option_list.Check_Item(1, MPlayerTiberium);
        option_list.Check_Item(2, MPlayerGoodies);
        //option_list.Check_Item(3, Special.IsShadowGrow);
        option_list.Check_Item(3, Special.IsCaptureTheFlag);

        auto& level_gauge = Get_Control<GaugeClass>(BUTTON_LEVEL);
        level_gauge.Set_Maximum(MPLAYER_BUILD_LEVEL_MAX - 1);
        level_gauge.Set_Value(BuildLevel - 1);

        auto& count_gauge = Get_Control<GaugeClass>(BUTTON_COUNT);
        count_gauge.Set_Maximum(MPlayerCountMax[MPlayerBases] - MPlayerCountMin[MPlayerBases]);
        count_gauge.Set_Value(MPlayerUnitCount - MPlayerCountMin[MPlayerBases]);

        auto& credits_gauge = Get_Control<GaugeClass>(BUTTON_CREDITS);
        credits_gauge.Set_Maximum(Rule.Get_Rule_Value<int>(GAME_MULTIPLAYER_SECTION, START_CREDITS_MAX_RULE));
        credits_gauge.Set_Value(MPlayerCredits);

        auto& tiberium_scale_gauge = Get_Control<GaugeClass>(BUTTON_TIBERIUM_SCALE);
        tiberium_scale_gauge.Set_Maximum(4);
        tiberium_scale_gauge.Set_Value(MPlayerTiberium < 2 ? 0 : MPlayerTiberium - 1);

        auto& scenario_list = Get_Control<ListClass>(BUTTON_SCENARIO_LIST);

        for (auto i = 0; i < MPlayerScenarios.Count(); i++) {
            scenario_list.Add_Item(strupr(MPlayerScenarios[i]));
        }
        ScenarioIdx = 0; // 1st scenario is selected

        // select the last scenario chosen by the player (if present)
        for (auto i = 0; i < MPlayerFilenum.Count(); i++) {
            if (MPlayerFilenum[i] == MPlayerScenarioNumber) {
                ScenarioIdx = i;
                scenario_list.Set_Selected_Index(i);
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
                Dimensions[BUTTON_OK].X,
                Dimensions[BUTTON_OK].Y,
                Dimensions[BUTTON_OK].W,
                Dimensions[BUTTON_OK].H
            )
        );
        Get_Control<TextButtonClass>(BUTTON_OK).Add_Tail(*CommandChain);

        Controls[BUTTON_CANCEL] = std::unique_ptr<GadgetClass>(
            new TextButtonClass(
                BUTTON_CANCEL,
                TXT_CANCEL,
                TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                Dimensions[BUTTON_CANCEL].X,
                Dimensions[BUTTON_CANCEL].Y,
                Dimensions[BUTTON_CANCEL].W,
                Dimensions[BUTTON_CANCEL].H
            )
        );
        Get_Control<TextButtonClass>(BUTTON_CANCEL).Add_Tail(*CommandChain);
    }

    void Init_Difficulty_Slider()
    {
        Controls[BUTTON_DIFFICULTY] = std::unique_ptr<GadgetClass>(
            new SliderClass(
                BUTTON_DIFFICULTY,
                Dimensions[BUTTON_NAME].X,
                Dimensions[BUTTON_OK].Y - (8 * Factor) - MarginWidth,
                Width - (Dimensions[BUTTON_NAME].X - X) * 2,
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
        // init gauges
        for (auto control = BUTTON_COUNT; control <= BUTTON_TIBERIUM_SCALE; ++control) {
            Controls[control] = std::unique_ptr<GadgetClass>(
                new GaugeClass(
                    control,
                    Dimensions[control].X,
                    Dimensions[control].Y,
                    Dimensions[control].W,
                    Dimensions[control].H
                )
            );
            Get_Control<GaugeClass>(control).Add_Tail(*CommandChain);
        }

        Controls[BUTTON_OPTIONS] = std::unique_ptr<GadgetClass>(
            new CheckListClass(
                BUTTON_OPTIONS,
                Dimensions[BUTTON_OPTIONS].X,
                Dimensions[BUTTON_OPTIONS].Y,
                Dimensions[BUTTON_OPTIONS].W,
                Dimensions[BUTTON_OPTIONS].H,
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
            const auto house_control = static_cast<ControlType>(control + 5);

            Text[control] = std::make_unique<char[]>(DropdownTextLength);
            Text[house_control] = std::make_unique<char[]>(DropdownTextLength);

            Controls[control] = std::unique_ptr<GadgetClass>(
                new DropListClass(
                    control,
                    Text[control].get(),
                    DropdownTextLength,
                    TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                    Dimensions[control].X,
                    Dimensions[control].Y,
                    Dimensions[control].W,
                    Dimensions[control].H,
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
                    Dimensions[house_control].X,
                    Dimensions[house_control].Y,
                    Dimensions[house_control].W,
                    Dimensions[house_control].H,
                    UpButtonShape,
                    DownButtonShape
                )
            );
            Get_Control<DropListClass>(house_control).Add_Tail(*CommandChain);
        }

        Controls[BUTTON_SCENARIO_LIST] = std::unique_ptr<GadgetClass>(
            new ListClass(
                BUTTON_SCENARIO_LIST,
                Dimensions[BUTTON_SCENARIO_LIST].X,
                Dimensions[BUTTON_SCENARIO_LIST].Y,
                Dimensions[BUTTON_SCENARIO_LIST].W,
                Dimensions[BUTTON_SCENARIO_LIST].H,
                TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                UpButtonShape,
                DownButtonShape
            )
        );
        Get_Control<ListClass>(BUTTON_SCENARIO_LIST).Add_Tail(*CommandChain);
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
                Dimensions[BUTTON_NAME].X,
                Dimensions[BUTTON_NAME].Y,
                Dimensions[BUTTON_NAME].W,
                Dimensions[BUTTON_NAME].H,
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
                Dimensions[BUTTON_HOUSE].X,
                Dimensions[BUTTON_HOUSE].Y,
                Dimensions[BUTTON_HOUSE].W,
                Dimensions[BUTTON_HOUSE].H,
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

        Dimensions[BUTTON_OK].W = 45 * Factor;
        Dimensions[BUTTON_OK].H = 9 * Factor;
        Dimensions[BUTTON_OK].X = X + (Width / 6) - (Dimensions[BUTTON_OK].W / 2);
        Dimensions[BUTTON_OK].Y = Y + Height - Dimensions[BUTTON_OK].H - MarginWidth - Factor * 6;

        Dimensions[BUTTON_CANCEL].W = 45 * Factor;
        Dimensions[BUTTON_CANCEL].H = 9 * Factor;
        Dimensions[BUTTON_CANCEL].X = X + Width - (Width / 6) - (Dimensions[BUTTON_CANCEL].W / 2);
        Dimensions[BUTTON_CANCEL].Y = Y + Height - Dimensions[BUTTON_CANCEL].H - MarginWidth - Factor * 6;

        Dimensions[BUTTON_NAME].W = 70 * Factor;
        Dimensions[BUTTON_NAME].H = 9 * Factor;
        Dimensions[BUTTON_NAME].X = X + 5 + (Width / 4) - (Dimensions[BUTTON_NAME].W / 2);
        Dimensions[BUTTON_NAME].Y = Y + MarginHeight + TextHeight + 1 * Factor;

        Dimensions[BUTTON_HOUSE].W = 60 * Factor;
        Dimensions[BUTTON_HOUSE].H = (3 * 5 * Factor);
        Dimensions[BUTTON_HOUSE].X = Center - (Dimensions[BUTTON_HOUSE].W / 2);
        Dimensions[BUTTON_HOUSE].Y = Dimensions[BUTTON_NAME].Y;

        Dimensions[BUTTON_COLOR_1].W = 10 * Factor;
        Dimensions[BUTTON_COLOR_1].H = 9 * Factor;
        Dimensions[BUTTON_COLOR_1].Y = Dimensions[BUTTON_NAME].Y;
        Dimensions[BUTTON_COLOR_1].X = X + ((Width / 4) * 3) - (Dimensions[BUTTON_COLOR_1].W * 3);

        for (auto control = BUTTON_COLOR_2; control <= BUTTON_COLOR_6; ++control) {
            Dimensions[control] = Dimensions[BUTTON_COLOR_1];
            Dimensions[control].X = Dimensions[control].X + (Dimensions[control].W * (control - BUTTON_COLOR_1));
        }

        Dimensions[BUTTON_PLAYER_LIST].W = 118 * Factor;
        Dimensions[BUTTON_PLAYER_LIST].X = X + MarginWidth + MarginWidth + 5 * Factor;

        Dimensions[BUTTON_SCENARIO_LIST].W = 140 * Factor;
        Dimensions[BUTTON_SCENARIO_LIST].H = 30 * Factor;
        Dimensions[BUTTON_SCENARIO_LIST].X =
            (Dimensions[BUTTON_CANCEL].X + static_cast<int>(nearbyint(Dimensions[BUTTON_CANCEL].W * 0.8)))
            - Dimensions[BUTTON_SCENARIO_LIST].W + (20 * Factor);
        Dimensions[BUTTON_SCENARIO_LIST].Y = Dimensions[BUTTON_COLOR_1].Y + TextHeight + 5 * Factor + TextHeight;

        Dimensions[BUTTON_SCENARIO_LIST].H *= 2;

        // right column of AI house controls
        Dimensions[BUTTON_AI_HOUSE_1].W = static_cast<int>(nearbyint(Dimensions[BUTTON_HOUSE].W / 1.75));
        Dimensions[BUTTON_AI_HOUSE_1].H = (6 * 5 * Factor);
        Dimensions[BUTTON_AI_HOUSE_1].X = Dimensions[BUTTON_SCENARIO_LIST].X
            - Dimensions[BUTTON_AI_HOUSE_1].W - (10 * Factor);
        Dimensions[BUTTON_AI_HOUSE_1].Y = Dimensions[BUTTON_SCENARIO_LIST].Y + (5 * Factor);

        for (auto control = BUTTON_AI_HOUSE_2; control <= BUTTON_AI_HOUSE_5; ++control) {
            const auto previous_control = static_cast<ControlType>(control - 1);

            Dimensions[control] = Dimensions[BUTTON_AI_HOUSE_1];
            Dimensions[control].Y = Dimensions[previous_control].Y + 10 * Factor;
        }

        // left column of AI house controls
        Dimensions[BUTTON_AI_DIFF_1].W = static_cast<int>(nearbyint(Dimensions[BUTTON_AI_HOUSE_1].W * 1.5));
        Dimensions[BUTTON_AI_DIFF_1].H = Dimensions[BUTTON_AI_HOUSE_1].H;
        Dimensions[BUTTON_AI_DIFF_1].X = Dimensions[BUTTON_AI_HOUSE_1].X
            - static_cast<int>(nearbyint(Dimensions[BUTTON_AI_HOUSE_1].W * 1.5)) - (10 * Factor);
        Dimensions[BUTTON_AI_DIFF_1].Y = Dimensions[BUTTON_AI_HOUSE_1].Y;

        for (auto control = BUTTON_AI_DIFF_2; control <= BUTTON_AI_DIFF_5; ++control) {
            const auto previous_control = static_cast<ControlType>(control - 1);

            Dimensions[control] = Dimensions[BUTTON_AI_DIFF_1];
            Dimensions[control].Y = Dimensions[previous_control].Y + 10 * Factor;
        }

        Dimensions[BUTTON_OPTIONS].W = static_cast<int>(nearbyint(Dimensions[BUTTON_SCENARIO_LIST].W * 0.8));
        Dimensions[BUTTON_OPTIONS].H = (5 * 6 * Factor) + 5 * Factor;
        Dimensions[BUTTON_OPTIONS].X = (Dimensions[BUTTON_SCENARIO_LIST].X + Dimensions[BUTTON_SCENARIO_LIST].W)
            - Dimensions[BUTTON_OPTIONS].W;
        Dimensions[BUTTON_OPTIONS].Y = Dimensions[BUTTON_SCENARIO_LIST].Y + Dimensions[BUTTON_SCENARIO_LIST].H
            + MarginWidth - 2 * Factor;

        Dimensions[BUTTON_COUNT].W = 25 * Factor;
        Dimensions[BUTTON_COUNT].H = 7 * Factor;
        Dimensions[BUTTON_COUNT].Y = Dimensions[BUTTON_OPTIONS].Y;
        Dimensions[BUTTON_COUNT].X = Dimensions[BUTTON_PLAYER_LIST].X + (Dimensions[BUTTON_PLAYER_LIST].W / 2)
            + 20 * Factor;

        Dimensions[BUTTON_LEVEL].W = 25 * Factor;
        Dimensions[BUTTON_LEVEL].H = 7 * Factor;
        Dimensions[BUTTON_LEVEL].Y = Dimensions[BUTTON_COUNT].Y + Dimensions[BUTTON_COUNT].H;
        Dimensions[BUTTON_LEVEL].X = Dimensions[BUTTON_PLAYER_LIST].X + (Dimensions[BUTTON_PLAYER_LIST].W / 2)
            + 20 * Factor;

        Dimensions[BUTTON_CREDITS].W = 25 * Factor;
        Dimensions[BUTTON_CREDITS].H = 7 * Factor;
        Dimensions[BUTTON_CREDITS].X = Dimensions[BUTTON_PLAYER_LIST].X + (Dimensions[BUTTON_PLAYER_LIST].W / 2)
            + 20 * Factor;
        Dimensions[BUTTON_CREDITS].Y = Dimensions[BUTTON_LEVEL].Y + Dimensions[BUTTON_LEVEL].H;

        Dimensions[BUTTON_TIBERIUM_SCALE].W = 25 * Factor;
        Dimensions[BUTTON_TIBERIUM_SCALE].H = 7 * Factor;
        Dimensions[BUTTON_TIBERIUM_SCALE].X = Dimensions[BUTTON_PLAYER_LIST].X + (Dimensions[BUTTON_PLAYER_LIST].W / 2)
            + 20 * Factor;
        Dimensions[BUTTON_TIBERIUM_SCALE].Y = Dimensions[BUTTON_CREDITS].Y + Dimensions[BUTTON_CREDITS].H;
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
        if (!Controls.contains(BUTTON_SCENARIO_LIST) || Controls[BUTTON_SCENARIO_LIST].get() == nullptr) {
            return;
        }

        auto& scenario_list = Get_Control<ListClass>(BUTTON_SCENARIO_LIST);

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

            Call_Back();
            Render(display);

            auto input = Get_Input(display);

            /*
            ---------------------------- Process input ----------------------------
            */
            switch (input) {
                /*------------------------------------------------------------------
                User clicks on a color button
                ------------------------------------------------------------------*/
                case KN_LMOUSE:
                    if (
                        Is_Mouse_Over_Rectangle(
                            Dimensions[BUTTON_COLOR_1].X,
                            Dimensions[BUTTON_COLOR_1].Y,
                            (Dimensions[BUTTON_COLOR_6].X + Dimensions[BUTTON_COLOR_6].W),
                            (Dimensions[BUTTON_COLOR_6].Y + Dimensions[BUTTON_COLOR_6].H)
                        )
                    ) {
                        MPlayerPrefColor = (Keyboard->MouseQX - Dimensions[BUTTON_COLOR_1].X)
                            / Dimensions[BUTTON_COLOR_1].W;
                        MPlayerColorIdx = MPlayerPrefColor;
                        display = REDRAW_COLORS;

                        auto& name_edt = Get_Control<EditClass>(BUTTON_NAME);
                        name_edt.Set_Color(MPlayerTColors[MPlayerColorIdx]);
                        name_edt.Flag_To_Redraw();

                        strcpy(MPlayerName, Text[BUTTON_NAME].get());

                        Collapse_Visible_Dropdowns(display);
                    }
                    break;

                    /*------------------------------------------------------------------
                    User edits the name field; retransmit new game options
                    ------------------------------------------------------------------*/
                case (BUTTON_NAME | KN_BUTTON): {
                    strcpy(MPlayerName, Text[BUTTON_NAME].get());

                    Collapse_Visible_Dropdowns(display);
                    break;
                }

                    /*------------------------------------------------------------------
                    House Buttons: set the player's desired House
                    ------------------------------------------------------------------*/
                case (BUTTON_HOUSE | KN_BUTTON): {
                    MPlayerHouse = static_cast<HousesType>(
                        Get_Control<DropListClass>(BUTTON_HOUSE).Current_Index() + HOUSE_GOOD
                    );
                    strcpy(MPlayerName, Text[BUTTON_NAME].get());

                    Collapse_Visible_Dropdowns(display);
                    display = REDRAW_BACKGROUND;
                    break;
                }

                    /*------------------------------------------------------------------
                    New Scenario selected.
                    ------------------------------------------------------------------*/
                case (BUTTON_SCENARIO_LIST | KN_BUTTON): {
                    auto& scenario_list = Get_Control<ListClass>(BUTTON_SCENARIO_LIST);

                    if (scenario_list.Current_Index() != ScenarioIdx) {
                        ScenarioIdx = scenario_list.Current_Index();

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

                        const auto house_btn = static_cast<ControlType>(control + 5);
                        auto& ai_diff_dropdown = Get_Control<DropListClass>(control);
                        auto& ai_house_dropdown = Get_Control<DropListClass>(house_btn);

                        // nothing changed, ignore input
                        if (!ai_diff_dropdown.List.Index_Changed()) {
                            break;
                        }

                        const auto diff_was_disabled = ai_diff_dropdown.Current_Index() == 0;
                        const auto diff_was_activated = ai_diff_dropdown.List.Get_Previous_Index() == 0;
                        const auto house_is_disabled = ai_house_dropdown.Current_Index() == 0;

                        if (diff_was_disabled) {
                            // reset house to match disabled AI diff
                            ai_house_dropdown.Set_Selected_Index(0);
                        } else if (diff_was_activated && house_is_disabled) {
                            // select a default house of '?' since none is selected
                            ai_house_dropdown.Set_Selected_Index(1);
                        }

                        ai_diff_dropdown.Collapse();
                        ai_house_dropdown.Collapse();
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

                        const auto diff_btn = static_cast<ControlType>(control - 5);
                        auto& ai_diff_dropdown = Get_Control<DropListClass>(diff_btn);
                        auto& ai_house_dropdown = Get_Control<DropListClass>(control);

                        // nothing changed, ignore input
                        if (!ai_house_dropdown.List.Index_Changed()) {
                            break;
                        }

                        const auto house_was_disabled = ai_house_dropdown.Current_Index() == 0;
                        const auto house_was_activated = ai_house_dropdown.List.Get_Previous_Index() == 0;
                        const auto diff_is_disabled = ai_diff_dropdown.Current_Index() == 0;

                        if (house_was_disabled) {
                            // reset diff to match disabled AI house
                            ai_diff_dropdown.Set_Selected_Index(0);
                        } else if (house_was_activated && diff_is_disabled) {
                            // select a default diff of 'Normal' since none is selected
                            ai_diff_dropdown.Set_Selected_Index(2);
                        }

                        ai_house_dropdown.Collapse();
                        ai_diff_dropdown.Collapse();
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
                    Collapse_Visible_Dropdowns(display);
                    break;
                }

                /*------------------------------------------------------------------
                User adjusts build level
                ------------------------------------------------------------------*/
                case (BUTTON_LEVEL | KN_BUTTON): {
                    BuildLevel = Get_Control<GaugeClass>(BUTTON_LEVEL).Get_Value() + 1;
                    if (BuildLevel > MPLAYER_BUILD_LEVEL_MAX) {
                        // if it's pegged, max it out
                        BuildLevel = MPLAYER_BUILD_LEVEL_MAX;
                    }

                    if (display < REDRAW_MESSAGE) {
                        display = REDRAW_MESSAGE;
                    }
                    Collapse_Visible_Dropdowns(display);
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
                    Collapse_Visible_Dropdowns(display);
                    break;
                }

                case (BUTTON_TIBERIUM_SCALE | KN_BUTTON): {
                    MPlayerTiberium = Get_Control<GaugeClass>(BUTTON_TIBERIUM_SCALE).Get_Value() + 1;

                    Get_Control<CheckListClass>(BUTTON_OPTIONS).Check_Item(1, MPlayerTiberium > 0);

                    Special.IsTGrowth = MPlayerTiberium;
                    Special.IsTSpread = MPlayerTiberium;

                    if (display < REDRAW_MESSAGE) {
                        display = REDRAW_MESSAGE;
                    }
                    Collapse_Visible_Dropdowns(display);
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
                    auto& option_list = Get_Control<CheckListClass>(BUTTON_OPTIONS);
                    auto count_gauge = Get_Control<GaugeClass>(BUTTON_COUNT);
                    auto tiberium_scale_gauge = Get_Control<GaugeClass>(BUTTON_TIBERIUM_SCALE);

                    if (MPlayerBases != option_list.Is_Checked(0)) {
                        MPlayerBases = option_list.Is_Checked(0);

                        if (MPlayerBases) {
                            MPlayerUnitCount = Fixed_To_Cardinal(
                                MPlayerCountMax[1] - MPlayerCountMin[1],
                                Cardinal_To_Fixed(
                                    MPlayerCountMax[0] - MPlayerCountMin[0],
                                    MPlayerUnitCount - MPlayerCountMin[0]
                                )
                            ) + MPlayerCountMin[1];
                        } else {
                            MPlayerUnitCount = Fixed_To_Cardinal(
                                MPlayerCountMax[0] - MPlayerCountMin[0],
                                Cardinal_To_Fixed(
                                    MPlayerCountMax[1] - MPlayerCountMin[1],
                                    MPlayerUnitCount - MPlayerCountMin[1]
                                )
                            ) + MPlayerCountMin[0];
                        }

                        count_gauge.Set_Maximum(MPlayerCountMax[MPlayerBases] - MPlayerCountMin[MPlayerBases]);
                        count_gauge.Set_Value(MPlayerUnitCount - MPlayerCountMin[MPlayerBases]);
                    }

                    MPlayerTiberium = option_list.Is_Checked(1) ? 1 : 0;

                    if (tiberium_scale_gauge.Get_Value() + 1 > 1) {
                        MPlayerTiberium = tiberium_scale_gauge.Get_Value() + 1;
                    }

                    tiberium_scale_gauge.Set_Value(MPlayerTiberium < 2 ? 0 : MPlayerTiberium - 1);

                    Special.IsTGrowth = MPlayerTiberium;
                    Special.IsTSpread = MPlayerTiberium;

                    MPlayerGoodies = option_list.Is_Checked(2);
                    Special.IsCaptureTheFlag = option_list.Is_Checked(3);

                    if (display < REDRAW_MESSAGE) {
                        display = REDRAW_MESSAGE;
                    }
                    Collapse_Visible_Dropdowns(display);
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
                    if (Messages.Get_Edit_Buf() != nullptr) {
                        Messages.Input(input);

                        if (display < REDRAW_MESSAGE) {
                            display = REDRAW_MESSAGE;
                        }

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

        switch (
            Get_Control<SliderClass>(BUTTON_DIFFICULTY).Get_Value() * (Rule.IsFineDifficulty ? 1 : 2)
        ) {
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

            default: break; // do nothing
        }

        // set AI player variables from difficulty/house dropdowns
        MPlayerGhosts = 0;

        for (auto control = BUTTON_AI_DIFF_1; control <= BUTTON_AI_DIFF_5; ++control) {
            const auto house_btn = static_cast<ControlType>(control + 5);
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

/**
 * Present a dialog to set up options for a Skirmish single player game.
 */
int Skirmish_Scenario_Dialog()
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
}
