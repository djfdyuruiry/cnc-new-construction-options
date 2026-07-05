#include <ranges>

#include "function.h"

#include "dialog.h"
#include "drop.h"
#include "typeconverter.h"

typedef enum
{
    SEARCH_BUTTON = 200,
    SEARCH_TEXTBOX,
    FILE_DROPDOWN,
    SECTION_DROPDOWN,
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

    void Iterate_Over_Rules_Page(
        std::function<
            void(const std::string_view&, const RuleValueVariant&, const std::string&, const RulesEditorControls&)
        > page_slot_handler,
        std::function<void(const RulesEditorControls&)> empty_page_slot_handler = [](const auto&){}
    )
    {
        const auto& active_rule_section = ActiveSectionsAreType
            ? Rule.Get_Type_Rules().at(ActionSectionsTypeName)[ActiveRuleSectionName]
            : Rule.Get_Rule_Sections()[ActiveRuleSectionName];

        const auto rule_names = active_rule_section.Rule_Names();
        const auto rule_count = rule_names.size();

        const auto offset = RulesPerPage * RulePageIndex;
        auto idx = offset;
        auto control = LEFT_RULE_VALUE_CONTROL;

        while (idx < offset + RulesPerPage && idx < rule_count) {
            auto rule_name = rule_names[idx];
            auto rule_value = active_rule_section.Get_Variant(rule_name);
            auto rule_string = RuleSection::Variant_To_String(rule_value);

            page_slot_handler(rule_name, rule_value, rule_string, control);

            idx++;
            ++control;

            if (control == LEFT_RULE_HELP_CONTROL) {
                control = RIGHT_RULE_VALUE_CONTROL;
            }
        }

        if (idx < offset + RulesPerPage && idx >= rule_count) {
            while (idx < offset + RulesPerPage) {
                empty_page_slot_handler(control);

                idx++;
                ++control;

                if (control == LEFT_RULE_HELP_CONTROL) {
                    control = RIGHT_RULE_VALUE_CONTROL;
                }
            }
        }
    }

    int Present_Unsaved_Changes_Prompt()
    {
        // TODO: Locale file entry
        return WWMessageBox().Process(
            "You have unsaved changes, you must either save or discard before navigating.",
            Text_String(TXT_SAVE_BUTTON),
            Text_String(TXT_CANCEL),
            "Discard"
        );
    }

    bool Unsaved_Changes_Present()
    {
        auto changes_present = false;

        Iterate_Over_Rules_Page(
            [&] (const auto& n, const auto& v, const auto& s, const auto& control) {
                changes_present = changes_present || Get_Control<EditClass>(control).Has_Changed();
            }
        );

        return changes_present;
    }

    /**
     * Resolve any unsaved changes by saving or discarding them. Returns false if player
     * refused to do either action.
     */
    bool Ensure_Unsaved_Changes_Resolved()
    {
        if (!Unsaved_Changes_Present()) {
            return true;
        }

        const auto prompt_result = Present_Unsaved_Changes_Prompt();

        if (prompt_result == 1) {
            // player discarded changes
            return true;
        }

        if (prompt_result == 2) {
            // player canceled
            return false;
        }

        return Save_Updated_Rules();
    }

    void Show_Update_Error_Popup(const std::invalid_argument& error)
    {
        // TODO: Locale file entry
        const auto full_message = std::format(
            "Unable to save your changes, check the values you entered are valid. ({})",
            error.what()
        );

        WWMessageBox().Process(full_message.c_str());
    }

    bool Save_Updated_Rules()
    {
        auto& active_rule_sections = ActiveSectionsAreType
            ? Rule.Get_Editable_Type_Rules()[ActionSectionsTypeName]
            : Rule.Get_Editable_Rule_Sections();
        auto& active_rule_section = active_rule_sections[ActiveRuleSectionName];

        auto update_succeeded = true;

        Iterate_Over_Rules_Page(
            [&] (const auto& name, const auto& value, const auto& value_string, const auto& control) {
                auto& edit_box = Get_Control<EditClass>(control);

                if (!edit_box.Has_Changed()) {
                    return;
                }

                const auto& type_name = active_rule_section.Get_Converter_Section_Type_Name();
                std::string new_value = Text[control].get();

                // rule is of special type that needs a non-trivial conversion from a string value
                if (ActiveSectionsAreType && TdTypeConverter::Rule_Requires_Converter(*type_name, name)) {
                    if (TdTypeConverter::Rule_Requires_Csv_Converter(*type_name, name)) {
                        const auto converter_variant = TdTypeConverter::Get_Csv_Rule_Variant(
                            *type_name,
                            name
                        );

                        try {
                            // convert string and set rule value
                            // (class_instance is updated by OnRulesChanged handler in section)
                            TdTypeConverter::Set_Csv_Rule_With_Variant(
                                active_rule_section,
                                name,
                                new_value,
                                converter_variant
                            );
                        } catch (const std::invalid_argument& ex) {
                            Show_Update_Error_Popup(ex);
                            update_succeeded = false;
                            return;
                        }
                    } else {
                        const auto converter_variant = TdTypeConverter::Get_Rule_Variant(
                            *type_name,
                            name
                        );

                        try {
                            // convert string and set rule value
                            // (class_instance is updated by OnRulesChanged handler in section)
                            TdTypeConverter::Set_Rule_With_Variant(
                                active_rule_section,
                                name,
                                new_value,
                                converter_variant
                            );
                        } catch (const std::invalid_argument& ex) {
                            Show_Update_Error_Popup(ex);
                            update_succeeded = false;
                            return;
                        }
                    }
                } else {
                    try {
                        const auto current_value = active_rule_section.Get_Variant(name);

                        // parse new value from edit box string
                        active_rule_section.Parse_String(name, new_value, current_value);
                    } catch (const std::invalid_argument& ex) {
                        Show_Update_Error_Popup(ex);
                        update_succeeded = false;
                        return;
                    }
                }

                // save rules back to INI file
                const auto rules_filename = Get_Control<FILE_DROPDOWN, DropListClass>().Current_Item();

                CCFileClass ini_file(rules_filename);
                INIClass ini;

                active_rule_sections.Save_All_To_Ini(ini);
                ini.Save(ini_file);
                ini_file.Close();

                // wait for player to edit again
                edit_box.Clear_Changed();
            }
        );

        return update_succeeded;
    }

    void Load_Current_Rules_Page()
    {
        /*
         * Load rule values for current page (up to RulesPerPage rules) into edit dialogs
         */
        Iterate_Over_Rules_Page(
            [&] (const auto& n, const auto& v, const auto& value_string, const auto& control) {
                // load rule value into edit control and enable
                auto edit_buffer = Text[control].get();

                strncpy(edit_buffer, value_string.c_str(), RuleValueTextLength);
                edit_buffer[RuleValueTextLength - 1] = '\0';

                Get_Control<EditClass>(control).Enable();
                Get_Control<EditClass>(control).Set_Text(edit_buffer, RuleValueTextLength);
            },
            [&] (auto& control) {
                // clear and disable unneeded controls
                strcpy(Text[control].get(), "");
                Get_Control<EditClass>(control).Disable(true);
                Get_Control<EditClass>(control).Set_Text(Text[control].get(), RuleValueTextLength);
            }
        );
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

    void Collapse_Visible_Dropdowns(DialogRedrawType& redraw_type)
    {
        auto& file_dropdown = Get_Control<FILE_DROPDOWN, DropListClass>();
        auto& section_dropdown = Get_Control<SECTION_DROPDOWN, DropListClass>();

        if (file_dropdown.IsDropped) {
            file_dropdown.Collapse();
            redraw_type = REDRAW_BACKGROUND;
        }

        if (section_dropdown.IsDropped) {
            section_dropdown.Collapse();
            redraw_type = REDRAW_BACKGROUND;
        }
    }

    void Set_Active_Rule_Section(std::string_view section)
    {
        ActiveRuleSectionName = section;

        const auto& active_rule_section = ActiveSectionsAreType
            ? Rule.Get_Type_Rules().at(ActionSectionsTypeName)[section]
            : Rule.Get_Rule_Sections()[section];

        // init pagination
        RulePageIndex = 0;
        RulePageCount = static_cast<int>(
            ceil(active_rule_section.Rule_Names().size() / static_cast<double>(RulesPerPage))
        );

        // load first page data
        Load_Current_Rules_Page();
    }

    void Set_Active_Rule_Sections(RuleSections& sections, std::optional<std::string_view> type_name = std::nullopt)
    {
        ActiveSectionsAreType = type_name.has_value();

        if (ActiveSectionsAreType) {
            ActionSectionsTypeName = *type_name;
        }

        RulePageIndex = 0;
        RulePageCount = 0;
        PreviousSectionIndex = 0;

        auto& section_dropdown = Get_Control<SECTION_DROPDOWN, DropListClass>();

        // clear any existing sections in the dropdown
        while (section_dropdown.List.Count() > 0) {
            section_dropdown.List.Remove_Item(0);
        }

        auto first = true;

        for (const auto& section_name : sections.Section_Names()) {
            // show player the first rule section
            if (first) {
                Set_Active_Rule_Section(section_name);
                first = false;
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

    bool ActiveSectionsAreType;
    std::string_view ActionSectionsTypeName;
    std::string_view ActiveRuleSectionName;
    int RulePageIndex;
    int RulePageCount;
    int PreviousFileIndex;
    int PreviousSectionIndex;

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

                if (Ensure_Unsaved_Changes_Resolved()) {
                    if (idx == 0) {
                        Set_Active_Rule_Sections(Rule.Get_Editable_Rule_Sections());
                    } else {
                        auto& type_rules = Rule.Get_Editable_Type_Rules();
                        auto type_key_idx = 0;

                        for (const auto& type_name : type_rules | std::ranges::views::keys) {
                            if (type_key_idx == idx - 1) {
                                Set_Active_Rule_Sections(type_rules[type_name], type_name);
                                break;
                            }

                            type_key_idx++;
                        }
                    }

                    PreviousFileIndex = idx;
                } else {
                    file_dropdown.Set_Selected_Index(PreviousFileIndex);
                }

                Collapse_Visible_Dropdowns(display);
                display = REDRAW_ALL;
                break;
            }

            case SECTION_DROPDOWN | KN_BUTTON: {
                auto& section_dropdown = Get_Control<SECTION_DROPDOWN, DropListClass>();

                if (Ensure_Unsaved_Changes_Resolved()) {
                    Set_Active_Rule_Section(section_dropdown.Current_Item());
                    PreviousSectionIndex = section_dropdown.Current_Index();
                } else {
                    section_dropdown.Set_Selected_Index(PreviousSectionIndex);
                }

                Collapse_Visible_Dropdowns(display);
                display = REDRAW_ALL;
                break;
            }

            case PREVIOUS_BUTTON | KN_BUTTON: {
                if (Ensure_Unsaved_Changes_Resolved()) {
                    Load_Previous_Rules_Page();
                }

                Collapse_Visible_Dropdowns(display);
                display = REDRAW_ALL;
                break;
            }

            case NEXT_BUTTON | KN_BUTTON: {
                if (Ensure_Unsaved_Changes_Resolved()) {
                    Load_Next_Rules_Page();
                }

                Collapse_Visible_Dropdowns(display);
                display = REDRAW_ALL;
                break;
            }

            case SAVE_BUTTON | KN_BUTTON: {
                Save_Updated_Rules();

                Collapse_Visible_Dropdowns(display);
                display = REDRAW_ALL;
                break;
            }

            case KN_ESC:
            case EXIT_BUTTON | KN_BUTTON: {
                Collapse_Visible_Dropdowns(display);

                if (Ensure_Unsaved_Changes_Resolved()) {
                    return false;
                }
                display = REDRAW_ALL;
            }

            default:
                break;
        }

        return std::nullopt;
    }

    KeyNumType Get_Input(DialogRedrawType& display) override
    {
        std::vector<RulesEditorControls> unfocused_controls;

        // collect edit box state before processing input
        for (auto control = LEFT_RULE_VALUE_CONTROL; control < RIGHT_RULE_HELP_CONTROL; ++control) {
            if (!Get_Control<EditClass>(control).Has_Focus()) {
                unfocused_controls.emplace_back(control);
            }

            if (control == LEFT_RULE_HELP_CONTROL - 1) {
                control = RIGHT_PANEL;
            }
        }

        auto file_was_dropped = Get_Control<FILE_DROPDOWN, DropListClass>().IsDropped;
        auto section_was_dropped = Get_Control<SECTION_DROPDOWN, DropListClass>().IsDropped;

        const auto input = Dialog::Get_Input(display);

        if (file_was_dropped && !Get_Control<FILE_DROPDOWN, DropListClass>().IsDropped) {
            display = REDRAW_ALL;
        }

        if (section_was_dropped && !Get_Control<SECTION_DROPDOWN, DropListClass>().IsDropped) {
            display = REDRAW_ALL;
        }

        if (input & KN_BUTTON) {
            // user is interacting with a button, so hide dropdown lists
            Collapse_Visible_Dropdowns(display);
        }

        // if player clicks on an edit box ensure dropdowns are collapsed
        for (auto control = LEFT_RULE_VALUE_CONTROL; control < RIGHT_RULE_HELP_CONTROL; ++control) {
            // check state changed to focused
            if (std::ranges::contains(unfocused_controls, control) && Get_Control<EditClass>(control).Has_Focus()) {
                Collapse_Visible_Dropdowns(display);
                break;
            }

            if (control == LEFT_RULE_HELP_CONTROL - 1) {
                control = RIGHT_PANEL;
            }
        }

        // for any visible dropdown, if mouse input is received outside it's bounds, hide it
        for (auto control = FILE_DROPDOWN; control <= SECTION_DROPDOWN; ++control) {
            auto& dropdown = Get_Control<DropListClass>(control);

            if (Is_Mouse_Outside_Dropdown_Dimensions(dropdown)) {
                if (dropdown.IsDropped) {
                    display = REDRAW_BACKGROUND;
                }

                dropdown.Collapse();
            }
        }

        return input;
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

        auto left_column_y = Dimensions[LEFT_PANEL].Y + VerticalSpacing;
        auto right_column_y = Dimensions[RIGHT_PANEL].Y + VerticalSpacing;

        Iterate_Over_Rules_Page(
            [&] (const auto& name, const auto& v, const auto& s, const auto& control) {
                auto x = control < RIGHT_RULE_VALUE_CONTROL ? Dimensions[LEFT_PANEL].X : Dimensions[RIGHT_PANEL].X;
                auto y = control < RIGHT_RULE_VALUE_CONTROL ? left_column_y : right_column_y;

                Fancy_Text_Print(name.data(),
                     x + HorizontalSpacing,
                     y,
                     CC_GREEN,
                     TBLACK,
                     TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

                if (control < RIGHT_RULE_VALUE_CONTROL) {
                    left_column_y += ControlHeight + VerticalSpacing + (10 * Factor);
                } else {
                    right_column_y += ControlHeight + VerticalSpacing + (10 * Factor);
                }
            }
        );
    }

    void Init_UI_State() override
    {
        PreviousFileIndex = 0;

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
