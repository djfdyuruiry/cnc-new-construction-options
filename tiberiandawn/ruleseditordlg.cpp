#include <ranges>

#include "function.h"

#include "dialog.h"
#include "drop.h"

typedef enum
{
    FILE_DROPDOWN = 200,
    SECTION_DROPDOWN,
    SEARCH_TEXTBOX,
    SEARCH_BUTTON,
    LEFT_PANEL, // virtual panel used for drawing
    LEFT_RULE_VALUE_CONTROL, // 7 controls
    LEFT_RULE_HELP_CONTROL = 212, // 7 controls
    RIGHT_PANEL = 219, // virtual panel used for drawing
    RIGHT_RULE_VALUE_CONTROL,  // 7 controls
    RIGHT_RULE_HELP_CONTROL = 227, // 7 controls
    PREVIOUS_BUTTON = 234,
    NEXT_BUTTON,
    EXIT_BUTTON,
    SAVE_BUTTON
} RulesEditorControls;

class RulesEditorDialog : public Dialog<RulesEditorControls>
{
    static constexpr auto DropdownTextLength = 25;
    static constexpr auto RuleValueTextLength = 45;
    static constexpr auto RulesPerPanel = 7;
    static constexpr auto RulesPerPage = RulesPerPanel * 2;

    void Load_Current_Rules_Page()
    {
        if (ActiveRuleSection == nullptr) {
            CNC_LOGGER_WARN("Attempted to load active rule section when pointer was null");
            return;
        }

        /*
         * Load rule values for current page (up to RulesPerPage rules) into edit dialogs
         */
        const auto rule_names = ActiveRuleSection->Rule_Names();
        const auto rule_count = rule_names.size();

        const auto offset = RulesPerPage * RulePageIndex;
        auto idx = offset;
        auto control = LEFT_RULE_VALUE_CONTROL;

        // left panel
        while (idx < offset + RulesPerPanel && idx < rule_count) {
            auto rule_string = RuleSection::Variant_To_String(
                ActiveRuleSection->Get_Variant(rule_names[idx])
            );

            // lowercase text will fit more characters on screen
            CncStringUtils::To_Lower(rule_string);

            // load rule value into edit control and enable
            auto edit_buffer = Text[control].get();

            strncpy(edit_buffer, rule_string.c_str(), RuleValueTextLength);
            edit_buffer[RuleValueTextLength - 1] = '\0';

            Get_Control<EditClass>(control).Enable();
            Get_Control<EditClass>(control).Set_Text(edit_buffer, RuleValueTextLength);

            idx++;
            ++control;
        }

        if (idx < offset + RulesPerPanel && idx >= rule_count) {
            while (idx < offset + RulesPerPanel) {
                // clear and disable unneeded controls
                strcpy(Text[control].get(), "");
                Get_Control<EditClass>(control).Disable(true);
                Get_Control<EditClass>(control).Set_Text(Text[control].get(), RuleValueTextLength);

                idx++;
                ++control;
            }
        }

        // right panel
        control = RIGHT_RULE_VALUE_CONTROL;

        while (idx < offset + RulesPerPage && idx < rule_count) {
            auto rule_string = RuleSection::Variant_To_String(
                ActiveRuleSection->Get_Variant(rule_names[idx])
            );

            // lowercase text will fit more characters on screen
            CncStringUtils::To_Lower(rule_string);

            // load rule value into edit control and enable
            auto edit_buffer = Text[control].get();

            strncpy(edit_buffer, rule_string.c_str(), RuleValueTextLength);
            edit_buffer[RuleValueTextLength - 1] = '\0';

            Get_Control<EditClass>(control).Enable();
            Get_Control<EditClass>(control).Set_Text(edit_buffer, RuleValueTextLength);

            idx++;
            ++control;
        }

        if (idx < offset + RulesPerPage && idx >= rule_count) {
            while (idx < offset + RulesPerPage) {
                // clear and disable unneeded controls
                strcpy(Text[control].get(), "");
                Get_Control<EditClass>(control).Disable(true);
                Get_Control<EditClass>(control).Set_Text(Text[control].get(), RuleValueTextLength);

                idx++;
                ++control;
            }
        }
    }

    bool Load_Previous_Rules_Page()
    {
        if (RulePageIndex < 1) {
            return false;
        }

        RulePageIndex--;
        Load_Current_Rules_Page();
        return true;
    }

    bool Load_Next_Rules_Page()
    {
        if (RulePageIndex == RulePageCount - 1) {
            return false;
        }

        RulePageIndex++;
        Load_Current_Rules_Page();
        return true;
    }

    void Set_Active_Rule_Section(RuleSection& section)
    {
        ActiveRuleSection = &section;

        // init pagination
        RulePageIndex = 0;
        RulePageCount = static_cast<int>(
            ceil(ActiveRuleSection->Rule_Names().size() / static_cast<double>(RulesPerPage))
        );

        // load first page data
        Load_Current_Rules_Page();
    }

    void Set_Active_Rule_Sections(RuleSections& sections)
    {
        ActiveRuleSections = &sections;
        ActiveRuleSection = nullptr;

        RulePageIndex = 0;
        RulePageCount = 0;

        auto& section_dropdown = Get_Control<SECTION_DROPDOWN, DropListClass>();

        // clear any existing sections in the dropdown
        while (section_dropdown.List.Count() > 0) {
            section_dropdown.List.Remove_Item(0);
        }

        for (const auto& section_name : sections.Section_Names()) {
            if (ActiveRuleSection == nullptr) {
                // show player the first rule section
                Set_Active_Rule_Section(sections[section_name]);
            }

            section_dropdown.Add_Item(section_name.data());
        }

        section_dropdown.Set_Selected_Index(0);
        section_dropdown.Set_Read_Only(true);
    }

    static void Init_Rule_File_Names_If_Empty()
    {
        if (!RuleFileNames.empty()) {
            return;
        }

        // prepare type rule file names
        std::vector<std::string> type_files;

        for (const auto& type_name : Rule.Get_Type_Rules() | std::views::keys) {
            type_files.emplace_back(std::format("{}.ini", type_name));
            CncStringUtils::To_Lower(type_files.back());
        }

        // add rules file names
        RuleFileNames.emplace_back(RulesClass::RulesFilename);
        CncStringUtils::To_Lower(RuleFileNames.back());

        for (const auto& type_file : type_files) {
            RuleFileNames.emplace_back(type_file);
        }
    }

    void Init_Bottom_Row()
    {
        Add_Button(PREVIOUS_BUTTON, "Previous");
        Add_Button(NEXT_BUTTON, "Next");
        Add_Button(EXIT_BUTTON, "Exit");
        Add_Button(SAVE_BUTTON, "Save");
    }

    void Init_Right_Rules_Panel()
    {
        for (auto control = RIGHT_RULE_VALUE_CONTROL; control < RIGHT_RULE_HELP_CONTROL; ++control) {
            Text[control] = std::make_unique<char[]>(RuleValueTextLength);
            Add_Control<EditClass>(
                control,
                Text[control].get(),
                RuleValueTextLength,
                TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                Dimensions[control].X,
                Dimensions[control].Y,
                Dimensions[control].W,
                Dimensions[control].H,
                EditClass::ALPHANUMERIC
            );
        }
    }

    void Init_Left_Rules_Panel()
    {
        for (auto control = LEFT_RULE_VALUE_CONTROL; control < LEFT_RULE_HELP_CONTROL; ++control) {
            Text[control] = std::make_unique<char[]>(RuleValueTextLength);
            Add_Control<EditClass>(
                control,
                Text[control].get(),
                RuleValueTextLength,
                TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                Dimensions[control].X,
                Dimensions[control].Y,
                Dimensions[control].W,
                Dimensions[control].H,
                EditClass::ALPHANUMERIC
            );
        }
    }

    void Init_Top_Row()
    {
        Add_Button(SEARCH_BUTTON, "Search");

        Text[SEARCH_TEXTBOX] = std::make_unique<char[]>(25);
        Add_Control<SEARCH_TEXTBOX, EditClass>(
            Text[SEARCH_TEXTBOX].get(),
            25,
            TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
            Dimensions[SEARCH_TEXTBOX].X,
            Dimensions[SEARCH_TEXTBOX].Y,
            Dimensions[SEARCH_TEXTBOX].W,
            Dimensions[SEARCH_TEXTBOX].H,
            EditClass::ALPHANUMERIC
        );

        Text[SECTION_DROPDOWN] = std::make_unique<char[]>(DropdownTextLength);
        Add_Control<SECTION_DROPDOWN, DropListClass>(
            Text[SECTION_DROPDOWN].get(),
            DropdownTextLength,
            TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
            Dimensions[SECTION_DROPDOWN].X,
            Dimensions[SECTION_DROPDOWN].Y,
            Dimensions[SECTION_DROPDOWN].W,
            Dimensions[SECTION_DROPDOWN].H,
            UpButtonShape,
            DownButtonShape
        );

        Text[FILE_DROPDOWN] = std::make_unique<char[]>(DropdownTextLength);
        Add_Control<FILE_DROPDOWN, DropListClass>(
            Text[FILE_DROPDOWN].get(),
            DropdownTextLength,
            TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
            Dimensions[FILE_DROPDOWN].X,
            Dimensions[FILE_DROPDOWN].Y,
            Dimensions[FILE_DROPDOWN].W,
            Dimensions[FILE_DROPDOWN].H,
            UpButtonShape,
            DownButtonShape
        );
    }

    static inline std::vector<std::string> RuleFileNames;

    RuleSections* ActiveRuleSections;
    RuleSection* ActiveRuleSection;
    int RulePageIndex;
    int RulePageCount;

    int ControlWidth;
    int ControlHeight;
    int DropdownItemHeight;
    int HorizontalSpacing;
    int VerticalSpacing;

    int ControlsX;
    int TopRowY;
    int MiddleRowY;
    int BottomRowY;

protected:
    std::optional<bool> On_Input(DialogRedrawType& display, KeyNumType& input) override
    {
        switch (input) {
            case FILE_DROPDOWN | KN_BUTTON: {
                auto& file_dropdown = Get_Control<FILE_DROPDOWN, DropListClass>();

                const auto idx = file_dropdown.Current_Index();

                if (idx == 0) {
                    Set_Active_Rule_Sections(Rule.Get_Editable_Rule_Sections());
                } else {
                    auto& type_rules = Rule.Get_Editable_Type_Rules();
                    auto type_key_idx = 0;

                    for (const auto& type_name : type_rules | std::ranges::views::keys) {
                        if (type_key_idx == idx - 1) {
                            Set_Active_Rule_Sections(type_rules[type_name]);
                            break;
                        }

                        type_key_idx++;
                    }
                }

                file_dropdown.Collapse();

                display = REDRAW_ALL;
                break;
            }

            case SECTION_DROPDOWN | KN_BUTTON: {
                auto& section_dropdown = Get_Control<SECTION_DROPDOWN, DropListClass>();

                std::string_view section = section_dropdown.Current_Item();

                if (ActiveRuleSections == nullptr) {
                    CNC_LOGGER_WARN("Attempted to load section from rule sections when it was null");
                    break;
                }

                Set_Active_Rule_Section((*ActiveRuleSections)[section]);
                section_dropdown.Collapse();

                display = REDRAW_ALL;
                break;
            }

            case PREVIOUS_BUTTON | KN_BUTTON: {
                // TODO: Check Has_Changed() on EditClass instances and prompt if unsaved changes (save/discard/cancel)
                Load_Previous_Rules_Page();
                display = REDRAW_ALL;
                break;
            }

            case NEXT_BUTTON | KN_BUTTON: {
                // TODO: Check Has_Changed() on EditClass instances and prompt if unsaved changes (save/discard/cancel)
                Load_Next_Rules_Page();
                display = REDRAW_ALL;
                break;
            }

            case SAVE_BUTTON | KN_BUTTON: {
                // TODO: Check Has_Changed() on EditClass instances and save each, with validation errors shown to user
                display = REDRAW_ALL;
                break;
            }

            case KN_RETURN:
            case EXIT_BUTTON | KN_BUTTON: {
                // TODO: Check Has_Changed() on EditClass instances and prompt if unsaved changes (save/discard/cancel)
                return false;
            }

            case KN_ESC: {
                return false;
            }

            default:
                break;
        }

        return std::nullopt;
    }

    void Render_Background(DialogRedrawType& display) override
    {
        Dialog::Render_Background(display);

        // panel borders
        for (const auto& panel : { LEFT_PANEL, RIGHT_PANEL }) {
            LogicPage->Draw_Rect(
                Dimensions[panel].X,
                Dimensions[panel].Y,
                Dimensions[panel].X + Dimensions[panel].W,
                Dimensions[panel].Y + Dimensions[panel].H,
                LTGRAY
            );
        }

        if (ActiveRuleSection == nullptr) {
            CNC_LOGGER_WARN("Attempted to render labels for rule section when it was null");
            return;
        }

        const auto rule_names = ActiveRuleSection->Rule_Names();
        const auto rule_count = rule_names.size();

        const auto offset = RulesPerPage * RulePageIndex;
        auto idx = offset;

        // left column rule labels
        auto left_column_y = Dimensions[LEFT_PANEL].Y + VerticalSpacing;

        while (idx < offset + RulesPerPanel && idx < rule_count) {
            Fancy_Text_Print(rule_names[idx].data(),
                 Dimensions[LEFT_PANEL].X + HorizontalSpacing,
                 left_column_y,
                 CC_GREEN,
                 TBLACK,
                 TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            left_column_y += ControlHeight + VerticalSpacing + (10 * Factor);

            idx++;
        }

        // right column rule labels
        auto right_column_y = Dimensions[RIGHT_PANEL].Y + VerticalSpacing;

        while (idx < offset + RulesPerPage && idx < rule_count) {
            Fancy_Text_Print(rule_names[idx].data(),
                 Dimensions[RIGHT_PANEL].X + HorizontalSpacing,
                 right_column_y,
                 CC_GREEN,
                 TBLACK,
                 TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

            right_column_y += ControlHeight + VerticalSpacing + (10 * Factor);

            idx++;
        }
    }

    void Init_UI_State() override
    {
        auto& file_dropdown = Get_Control<FILE_DROPDOWN, DropListClass>();

        for (const auto& file : RuleFileNames) {
            file_dropdown.Add_Item(file.data());
        }

        // default to rules.ini
        file_dropdown.Set_Selected_Index(0);
        file_dropdown.Set_Read_Only(true);

        // default to show rules.ini sections
        Set_Active_Rule_Sections(Rule.Get_Editable_Rule_Sections());
    }

    void Init_Data() override
    {
        Init_Rule_File_Names_If_Empty();
    }

    void Init_Controls() override
    {
        Dialog::Init_Controls();

        Init_Top_Row();
        Init_Left_Rules_Panel();
        Init_Right_Rules_Panel();
        Init_Bottom_Row();
    }

    //  TODO: use String_Pixel_Width() more
    void Init_Dimensions(const int screen_width, const int screen_height, const int factor) override
    {
        Dialog::Init_Dimensions(screen_width, screen_height, factor);

        ControlWidth = 40 * Factor;
        ControlHeight = 9 * Factor;
        DropdownItemHeight = 8 * Factor;
        HorizontalSpacing = 3 * Factor;
        VerticalSpacing = 2 * Factor;

        ControlsX = X + MarginWidth;
        TopRowY = Y + MarginHeight + (13 * Factor);
        MiddleRowY = TopRowY + ControlHeight + VerticalSpacing;
        BottomRowY = Y + Height - ControlHeight - MarginHeight;

        // top row
        Dimensions[FILE_DROPDOWN]= {
            ControlsX,
            TopRowY,
            static_cast<int>(nearbyint(ControlWidth * 1.2)),
            5 * DropdownItemHeight // 5 visible items (excluding selected value)
        };

        Dimensions[SECTION_DROPDOWN] = {
            ControlsX + Dimensions[FILE_DROPDOWN].W + (HorizontalSpacing * 4),
            TopRowY,
            static_cast<int>(nearbyint(ControlWidth * 1.75)),
            10 * DropdownItemHeight // 10 visible items (excluding selected value)
        };

        Dimensions[SEARCH_BUTTON] = {
            X + Width - ControlWidth - MarginWidth,
            TopRowY,
            ControlWidth,
            ControlHeight
        };

        Dimensions[SEARCH_TEXTBOX] = {
            Dimensions[SEARCH_BUTTON].X - (ControlWidth * 2) - HorizontalSpacing,
            TopRowY,
            ControlWidth * 2,
            ControlHeight
        };

        // panels (middle row)
        const auto panel_width = ((Width - (MarginWidth * 2)) - VerticalSpacing) / 2 - 1;
        const auto panel_height = BottomRowY - MiddleRowY - VerticalSpacing;

        // left panel
        Dimensions[LEFT_PANEL] = {
            ControlsX,
            MiddleRowY,
            panel_width,
            panel_height
        };

        auto left_control_y = Dimensions[LEFT_PANEL].Y;

        for (auto control = LEFT_RULE_VALUE_CONTROL; control < LEFT_RULE_HELP_CONTROL; ++control) {
            left_control_y += VerticalSpacing + (10 * Factor);

            Dimensions[control] = {
                Dimensions[LEFT_PANEL].X + HorizontalSpacing,
                left_control_y,
                panel_width - (HorizontalSpacing * 2),
                ControlHeight
            };

            left_control_y += ControlHeight;
        }

        // right panel
        Dimensions[RIGHT_PANEL] = {
            ControlsX + panel_width + VerticalSpacing,
            MiddleRowY,
            panel_width,
            panel_height
        };

        auto right_control_y = Dimensions[RIGHT_PANEL].Y;

        for (auto control = RIGHT_RULE_VALUE_CONTROL; control < RIGHT_RULE_HELP_CONTROL; ++control) {
            right_control_y += VerticalSpacing + (10 * Factor);

            Dimensions[control] = {
                Dimensions[RIGHT_PANEL].X + HorizontalSpacing,
                right_control_y,
                panel_width - (HorizontalSpacing * 2),
                ControlHeight
            };

            right_control_y += ControlHeight;
        }

        // bottom row
        Dimensions[EXIT_BUTTON] = {ControlsX, BottomRowY, ControlWidth, ControlHeight};

        Dimensions[SAVE_BUTTON] = {
            ControlsX + ControlWidth + VerticalSpacing,
            BottomRowY,
            ControlWidth,
            ControlHeight
        };

        Dimensions[NEXT_BUTTON] = {
            X + Width - ControlWidth - MarginWidth,
            BottomRowY,
            ControlWidth,
            ControlHeight
        };

        Dimensions[PREVIOUS_BUTTON] = {
            Dimensions[NEXT_BUTTON].X - ControlWidth - HorizontalSpacing,
            BottomRowY,
            ControlWidth,
            ControlHeight
        };
    }

public:
    RulesEditorDialog() : Dialog(300, 195, 5, 5),
        ActiveRuleSections(nullptr),
        ActiveRuleSection(nullptr),
        RulePageIndex(0),
        RulePageCount(0)
    {
        CaptionText = "Rules Editor";
    }
};

bool Rules_Editor_Dialog()
{
    const auto factor = (SeenBuff.Get_Width() == 320) ? 1 : 2;

    RulesEditorDialog dialog;

    dialog.Init(
        Try_Get_Resolution_Mode_Width().value_or(SeenBuff.Get_Width()),
        Try_Get_Resolution_Mode_Height().value_or(SeenBuff.Get_Height()),
        factor
    );

    return dialog.Present();
}
