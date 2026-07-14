#include <ranges>

#include "function.h"

#include "dialog.h"
#include "drop.h"
#include "typeconverter.h"

typedef enum
{
    VALUE_DROPDOWN = 200,
    CR_HELP_BUTTON,
    CR_SAVE_BUTTON,
    CR_CANCEL_BUTTON
} ConverterRuleEditorControls;

class ConverterRuleEditorDialog : public Dialog<ConverterRuleEditorControls>
{
    RuleSection& Section;
    const std::string& RuleName;
    std::vector<std::string> ValidValues;

protected:
    void Init_UI_State() override
    {
    }

    void Init_Data() override
    {
        const auto rule_variant = Section.Get_Variant(RuleName);
        const auto string_value = RuleSection::Variant_To_String(rule_variant);

        const auto converter_variant = TdTypeConverter::Get_Rule_Variant(
            *Section.Get_Converter_Section_Type_Name(),
            RuleName
        );
        auto& dropdown = Get_Control<VALUE_DROPDOWN, DropListClass>();
        auto value_index = 0;

        ValidValues = TdTypeConverter::Get_Valid_Strings_Variant(converter_variant);

        for (const auto& str : ValidValues) {
            dropdown.Add_Item(str.c_str());

            // select the current rule value
            if (str == string_value) {
                value_index = dropdown.Count() - 1;
            }
        }

        dropdown.Set_Selected_Index(value_index);
    }

    void Init_Dimensions(const int screen_width, const int screen_height, const int factor) override
    {
        Dialog::Init_Dimensions(screen_width, screen_height, factor);

        Dimensions[VALUE_DROPDOWN] = {
            X + MarginWidth,
            Y + MarginHeight + 25 * factor,
            100 * factor,
            8 * (8 * Factor) // 8 visible items
        };
        Dimensions[CR_SAVE_BUTTON] = {
            X + Width - MarginWidth - 30 * factor,
            Y + Height - MarginHeight - 10 * factor,
            30 * factor,
            10 * factor
        };
        Dimensions[CR_CANCEL_BUTTON] = {
            X + MarginWidth,
            Y + Height - MarginHeight - 10 * factor,
            30 * factor,
            10 * factor
        };
    }

    void Init_Controls() override
    {
        Dialog::Init_Controls();

        Add_Button(CR_CANCEL_BUTTON, "Cancel");
        Add_Button(CR_SAVE_BUTTON, "Save");

        Text[VALUE_DROPDOWN] = std::make_unique<char[]>(25);
        Add_Control<VALUE_DROPDOWN, DropListClass>(
            Text[VALUE_DROPDOWN].get(),
            25,
            TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
            Dimensions[VALUE_DROPDOWN].X,
            Dimensions[VALUE_DROPDOWN].Y,
            Dimensions[VALUE_DROPDOWN].W,
            Dimensions[VALUE_DROPDOWN].H,
            UpButtonShape,
            DownButtonShape
        );
    }

    std::optional<bool> On_Input(DialogRedrawType& display, KeyNumType& input) override
    {
        switch (input) {
            case (VALUE_DROPDOWN | KN_BUTTON): {
                Get_Control<VALUE_DROPDOWN, DropListClass>().Collapse();
                display = REDRAW_ALL;
                break;
            }

            case CR_SAVE_BUTTON | KN_BUTTON: {
                auto& dropdown = Get_Control<VALUE_DROPDOWN, DropListClass>();
                const std::string selected_value = dropdown.Current_Item();

                const auto variant = TdTypeConverter::Get_Rule_Variant(
                    *Section.Get_Converter_Section_Type_Name(),
                    RuleName
                );

                TdTypeConverter::Set_Rule_With_Variant(Section, RuleName, selected_value, variant);
                return true;
            }

            case KN_ESC:
            case CR_CANCEL_BUTTON | KN_BUTTON: {
                return false;
            }

            default: break;
        }

        return std::nullopt;
    }

    KeyNumType Get_Input(DialogRedrawType& display) override
    {
        static auto previous_dropped_value = false;

        auto& value_dropdown = Get_Control<VALUE_DROPDOWN, DropListClass>();
        const auto was_dropped = value_dropdown.IsDropped;

        // hide buttons when dropdown is visible to prevent input conflicts
        if (!previous_dropped_value && was_dropped) {
            Get_Control<CR_CANCEL_BUTTON, TextButtonClass>().Disable(true);
            Get_Control<CR_SAVE_BUTTON, TextButtonClass>().Disable(true);
        } else if (previous_dropped_value && !was_dropped) {
            Get_Control<CR_CANCEL_BUTTON, TextButtonClass>().Enable();
            Get_Control<CR_SAVE_BUTTON, TextButtonClass>().Enable();
        }

        previous_dropped_value = was_dropped;

        const auto input = Dialog::Get_Input(display);

        if (!was_dropped && value_dropdown.IsDropped) {
            // BUG: workaround for scrollbars not rendering reliably
            value_dropdown.List.Redraw_Scroll_Bar(true);
        }

        return input;
    }

    void Render_Background(DialogRedrawType& display) override
    {
        Dialog::Render_Background(display);

        const auto variant = TdTypeConverter::Get_Rule_Variant(
            *Section.Get_Converter_Section_Type_Name(),
            RuleName
        );
        const auto variant_name = TdTypeConverter::Get_Type_Name_Variant(variant);

        Fancy_Text_Print(std::format("Rule type: {}", variant_name.data()).c_str(),
                        X + MarginWidth,
                        Y + MarginHeight + 15 * Factor,
                        CC_GREEN,
                        TBLACK,
                        TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
    }

public:
    ConverterRuleEditorDialog(RuleSection& section, const std::string& rule_name)
        : Dialog(120, 120, 5, 5),
          Section(section),
          RuleName(rule_name)
    {
        CaptionText = std::format("{}: {}", section.Get_Section_Name(), RuleName);
    }
};

typedef enum
{
    VALUES_CHECKLIST = 200,
    CCR_HELP_BUTTON,
    CCR_SAVE_BUTTON,
    CCR_CANCEL_BUTTON
} ConverterCsvRuleEditorControls;

class ConverterCsvRuleEditorDialog : public Dialog<ConverterCsvRuleEditorControls>
{
    RuleSection& Section;
    const std::string& RuleName;
    std::vector<std::string> ValidValues;

protected:

    void Init_UI_State() override
    {
    }

    void Init_Data() override
    {
        const auto csv_variant = TdTypeConverter::Get_Csv_Rule_Variant(
            *Section.Get_Converter_Section_Type_Name(),
            RuleName
        );

        auto& checklist = Get_Control<VALUES_CHECKLIST, CheckListClass>();

        ValidValues = TdTypeConverter::Get_Valid_Strings_Variant(csv_variant);

        for (const auto& str : ValidValues) {
            checklist.Add_Item(str.data());
        }

        const auto current_value_csv = RuleSection::Variant_To_String(Section.Get_Variant(RuleName));

        // Set initial checked state
        for (auto i = 0; i < checklist.Count(); ++i) {
            const auto item_text = checklist.Get_Item(i);

            if (current_value_csv.find(item_text) != std::string::npos) {
                checklist.Check_Item(i, true);
            }
        }
    }

    void Init_Dimensions(const int screen_width, const int screen_height, const int factor) override
    {
        Dialog::Init_Dimensions(screen_width, screen_height, factor);

        Dimensions[CCR_SAVE_BUTTON] = {
            X + Width - MarginWidth - 30 * factor,
            Y + Height - MarginHeight - 10 * factor,
            30 * factor,
            10 * factor
        };
        Dimensions[CCR_CANCEL_BUTTON] = {
            X + MarginWidth,
            Y + Height - MarginHeight - 10 * factor,
            30 * factor,
            10 * factor
        };

        Dimensions[VALUES_CHECKLIST].X = X + MarginWidth;
        Dimensions[VALUES_CHECKLIST].Y = Y + MarginHeight + (20 * Factor);
        Dimensions[VALUES_CHECKLIST].W = Width - (MarginWidth * 2);
        Dimensions[VALUES_CHECKLIST].H = Dimensions[CCR_SAVE_BUTTON].X - (5 * Factor) - Dimensions[VALUES_CHECKLIST].X;
    }

    void Init_Controls() override
    {
        Dialog::Init_Controls();

        Add_Button(CCR_CANCEL_BUTTON, "Cancel");
        Add_Button(CCR_SAVE_BUTTON, "Save");

        Add_Control<VALUES_CHECKLIST, CheckListClass>(
            Dimensions[VALUES_CHECKLIST].X,
            Dimensions[VALUES_CHECKLIST].Y,
            Dimensions[VALUES_CHECKLIST].W,
            Dimensions[VALUES_CHECKLIST].H,
            TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
            UpButtonShape,
            DownButtonShape
        );
    }

    std::optional<bool> On_Input(DialogRedrawType& display, KeyNumType& input) override
    {
        switch (input) {
            case CCR_SAVE_BUTTON | KN_BUTTON: {
                const auto& checklist = Get_Control<VALUES_CHECKLIST, CheckListClass>();
                std::vector<std::string> selected_values;

                for (int i = 0; i < checklist.Count(); ++i) {
                    if (checklist.Is_Checked(i)) {
                        selected_values.emplace_back(checklist.Get_Item(i));
                    }
                }

                if (selected_values.empty()) {
                    WWMessageBox().Process("You must check at least one value in the list");
                    break;
                }

                const auto csv_value = CncStringUtils::To_Csv(selected_values);
                const auto variant = TdTypeConverter::Get_Csv_Rule_Variant(
                    *Section.Get_Converter_Section_Type_Name(),
                    RuleName
                );

                TdTypeConverter::Set_Csv_Rule_With_Variant(Section, RuleName, csv_value, variant);
                return true;
            }

            case KN_ESC:
            case CCR_CANCEL_BUTTON | KN_BUTTON: {
                return false;
            }

            default: break;
        }

        return std::nullopt;
    }

    void Render_Background(DialogRedrawType& display) override
    {
        Dialog::Render_Background(display);

        const auto variant = TdTypeConverter::Get_Csv_Rule_Variant(
            *Section.Get_Converter_Section_Type_Name(),
            RuleName
        );
        const auto& variant_name = TdTypeConverter::Get_Type_Name_Variant(variant);

        Fancy_Text_Print(std::format("Rule type: {}", variant_name.data()).c_str(),
                        Dimensions[VALUES_CHECKLIST].X,
                        Dimensions[VALUES_CHECKLIST].Y - (10 * Factor),
                        CC_GREEN,
                        TBLACK,
                        TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
    }

public:
    ConverterCsvRuleEditorDialog(RuleSection& section, const std::string& rule_name)
        : Dialog(150, 150, 5, 5),
          Section(section),
          RuleName(rule_name)
    {
        CaptionText = std::format("{}: {}", section.Get_Section_Name(), RuleName);
    }
};

typedef enum
{
    SEARCH_BUTTON = 200,
    SEARCH_TEXTBOX,
    FILE_DROPDOWN,
    SECTION_DROPDOWN,
    LEFT_PANEL, // virtual panel used for drawing
    LEFT_RULE_VALUE_CONTROL, // 7 controls
    LEFT_RULE_HELP_CONTROL = 212, // 7 controls
    LEFT_RULE_EDIT_BUTTON = 219, // 7 controls
    RIGHT_PANEL = 226, // virtual panel used for drawing
    RIGHT_RULE_VALUE_CONTROL,  // 7 controls
    RIGHT_RULE_HELP_CONTROL = 234, // 7 controls
    RIGHT_RULE_EDIT_BUTTON = 241, // 7 controls
    PREVIOUS_BUTTON = 248,
    NEXT_BUTTON,
    EXIT_BUTTON,
    LOAD_DEFAULTS_BUTTON,
    SAVE_CHANGES_BUTTON
} RulesEditorControls;

class RulesEditorDialog : public Dialog<RulesEditorControls>
{
    static constexpr auto DropdownTextLength = 25;
    static constexpr auto RuleValueTextLength = 45;
    static constexpr auto RulesPerPanel = 7;
    static constexpr auto RulesPerPage = RulesPerPanel * 2;

    /**
     * Group of controls that represent a rule value on the screen.
     */
    struct RuleControls
    {
        RulesEditorControls edit_button;
        RulesEditorControls help_button;
        RulesEditorControls value_control;
    };

    RuleSections& Get_Active_Rule_Sections() const
    {
        return ActiveSectionsAreType
            ? Rule.Get_Editable_Type_Rules().at(ActionSectionsTypeName)
            : Rule.Get_Editable_Rule_Sections();
    }

    void Iterate_Over_Rules_Page(
        const std::function<void(RuleSection&, const std::string&, const RuleControls&)>& page_slot_handler,
        const std::function<void(const RuleControls&)>& empty_page_slot_handler = [](const auto&){}
    ) const
    {
        auto& active_rule_section = Get_Active_Rule_Sections().Get_Section(ActiveRuleSectionName);

        const auto rule_names = active_rule_section.Rule_Names();
        const auto rule_count = rule_names.size();

        const auto offset = RulesPerPage * RulePageIndex;
        auto idx = offset;
        auto control = LEFT_RULE_VALUE_CONTROL;
        auto edit_btn = LEFT_RULE_EDIT_BUTTON;
        auto help_btn = LEFT_RULE_HELP_CONTROL;

        while (idx < offset + RulesPerPage && idx < rule_count) {
            const std::string rule_name = rule_names[idx].data();

            page_slot_handler(active_rule_section, rule_name, { edit_btn, help_btn, control});

            idx++;
            ++control;
            ++edit_btn;
            ++help_btn;

            if (control == LEFT_RULE_HELP_CONTROL) {
                control = RIGHT_RULE_VALUE_CONTROL;
                edit_btn = RIGHT_RULE_EDIT_BUTTON;
                help_btn = RIGHT_RULE_HELP_CONTROL;
            }
        }

        if (idx < offset + RulesPerPage && idx >= rule_count) {
            while (idx < offset + RulesPerPage) {
                empty_page_slot_handler({ edit_btn, help_btn, control});

                idx++;
                ++control;
                ++edit_btn;
                ++help_btn;

                if (control == LEFT_RULE_HELP_CONTROL) {
                    control = RIGHT_RULE_VALUE_CONTROL;
                    edit_btn = RIGHT_RULE_EDIT_BUTTON;
                    help_btn = RIGHT_RULE_HELP_CONTROL;
                }
            }
        }
    }

    static bool Delete_INI_File_If_Exists(const char* file_name)
    {
        if (CCFileClass(file_name).Delete()) {
            // file delete was successful
            return true;
        }

        WWMessageBox().Process(
            std::format("Error deleting existing INI file: {}", file_name).c_str(),
            Text_String(TXT_OK)
        );
        return false;
    }

    void Load_Defaults()
    {
        auto& file_dropdown = Get_Control<FILE_DROPDOWN, DropListClass>();

        // confirmation popup
        const auto file_name = file_dropdown.Current_Item();
        const auto confirm_msg = std::format(
            "Do you want to reset all rule INI files or only '{}'?",
            file_name
        );
        const auto only_button_text = std::format("Only Reset '{}'", file_name);

        const auto response = WWMessageBox().Process(
            confirm_msg.c_str(),
            Text_String(TXT_CANCEL),
            "Reset All Files",
            only_button_text.c_str()
        );

        if (response == 0) {
            return;
        }

        if (response == 1) {
            // purge just the current INI file
            if (!Delete_INI_File_If_Exists(file_dropdown.Current_Item())) {
                return;
            }
        } else {
            // purge all existing INI files
            for (auto i = 0; i < file_dropdown.Count(); ++i) {
                if (!Delete_INI_File_If_Exists(file_dropdown.Get_Item(i))) {
                    return;
                }
            }
        }

        // init method will write fresh INI files
        Rule.Init();

        /**
         * Refresh UI to ensure UI matches loaded defaults
         */
        const auto active_section = ActiveRuleSectionName;
        const auto active_rule_page = RulePageIndex;

        // refresh sections list
        ActiveSectionsAreType
            ? Set_Active_Rule_Sections(Get_Active_Rule_Sections(), ActionSectionsTypeName)
            : Set_Active_Rule_Sections(Get_Active_Rule_Sections());

        // refresh active section selection
        auto& section_dropdown = Get_Control<SECTION_DROPDOWN, DropListClass>();
        for (auto i = 0; i < section_dropdown.Count(); ++i) {
            if (active_section == section_dropdown.Get_Item(i) ) {
                section_dropdown.Set_Selected_Index(i);
                break;
            }
        }

        // refresh active section
        Set_Active_Rule_Section(active_section);

        // navigate to correct page
        RulePageIndex = active_rule_page;
        Load_Current_Rules_Page();
    }

    static int Present_Unsaved_Changes_Prompt()
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
            [&] (const auto& s, const auto& n, const auto& controls) {
                changes_present = changes_present || Get_Control<EditClass>(controls.value_control).Has_Changed();
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

    static void Show_Update_Error_Popup(const std::invalid_argument& error)
    {
        // TODO: Locale file entry
        const auto full_message = std::format(
            "Unable to save your changes, check the values you entered are valid. ({})",
            error.what()
        );

        WWMessageBox().Process(full_message.c_str());
    }

    void Update_Ini_File()
    {
        // save rules back to INI file
        const auto rules_filename = Get_Control<FILE_DROPDOWN, DropListClass>().Current_Item();

        CCFileClass ini_file(rules_filename);
        INIClass ini;

        Get_Active_Rule_Sections().Save_All_To_Ini(ini);
        ini.Save(ini_file);
        ini_file.Close();
    }

    void Save_Updated_Rule(
        RuleSection& section,
        const std::string_view& name,
        const RulesEditorControls& control
    )
    {
        const std::string new_value = Text[control].get();

        // rule is of special type that needs a non-trivial conversion from a string value
        if (ActiveSectionsAreType && TdTypeConverter::Rule_Requires_Converter(section, name)) {
            const auto& type_name = section.Get_Converter_Section_Type_Name();

            if (TdTypeConverter::Rule_Requires_Csv_Converter(section, name)) {
                const auto csv_converter_variant = TdTypeConverter::Get_Csv_Rule_Variant(
                    *type_name,
                    name
                );

                // convert string and set rule value
                // (class_instance is updated by OnRulesChanged handler in section)
                TdTypeConverter::Set_Csv_Rule_With_Variant(
                    section,
                    name,
                    new_value,
                    csv_converter_variant
                );
            } else {
                const auto converter_variant = TdTypeConverter::Get_Rule_Variant(
                    *type_name,
                    name
                );

                // convert string and set rule value
                // (class_instance is updated by OnRulesChanged handler in section)
                TdTypeConverter::Set_Rule_With_Variant(
                    section,
                    name,
                    new_value,
                    converter_variant
                );
            }
        } else {
            const auto current_value = section.Get_Variant(name);

            // parse new value from edit box string
            section.Parse_String(name, new_value, current_value);
        }

        Update_Ini_File();

        // wait for player to edit again
        Get_Control<EditClass>(control).Clear_Changed();
    }

    bool Save_Updated_Rules()
    {
        auto update_succeeded = true;

        Iterate_Over_Rules_Page(
            [&] (auto& section, const auto& name, const auto& controls) {
                if (Get_Control<EditClass>(controls.value_control).Has_Changed()) {
                    try {
                        Save_Updated_Rule(section, name, controls.value_control);
                    } catch (const std::invalid_argument& ex) {
                        Show_Update_Error_Popup(ex);
                        update_succeeded = false;
                    }
                }
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
            [&] (const auto& section, const auto& name, const auto& controls) {
                // load rule value into edit control and enable
                const auto& [edit_button, help_button, value_control] = controls;

                auto edit_buffer = Text[controls.value_control].get();
                auto rule_variant = section.Get_Variant(name);
                auto value_string = RuleSection::Variant_To_String(rule_variant);

                if (std::holds_alternative<bool>(rule_variant)) {
                    const auto bool_value = std::get<bool>(rule_variant);
                    value_string = bool_value ? "yes" : "no";
                }

                strncpy(edit_buffer, value_string.c_str(), RuleValueTextLength);
                edit_buffer[RuleValueTextLength - 1] = '\0';

                Get_Control<EditClass>(value_control).Set_Text(edit_buffer, RuleValueTextLength);

                if (ActiveSectionsAreType && TdTypeConverter::Rule_Requires_Converter(section, name)) {
                    // don't allow player to manually edit converter type values, show an edit button instead
                    Get_Control<EditClass>(value_control).Disable();
                    Get_Control<TextButtonClass>(edit_button).Enable();
                } else {
                    Get_Control<EditClass>(value_control).Enable();
                    Get_Control<TextButtonClass>(edit_button).Disable(true);
                }

                Get_Control<TextButtonClass>(help_button).Enable();
            },
            [&] (const auto& controls) {
                // clear and disable unneeded controls
                const auto& [edit_button, help_button, value_control] = controls;

                strcpy(Text[value_control].get(), "");
                Get_Control<EditClass>(value_control).Disable(true);
                Get_Control<EditClass>(value_control).Set_Text(Text[value_control].get(), RuleValueTextLength);
                Get_Control<TextButtonClass>(edit_button).Disable(true);
                Get_Control<TextButtonClass>(help_button).Disable(true);
            }
        );
    }

    bool Load_Previous_Rules_Page()
    {
        if (RulePageIndex < 1) {
            return false;
        }

        if (RulePageIndex - 1 < 1) {
            Get_Control<PREVIOUS_BUTTON, TextButtonClass>().Disable();
        } else {
            Get_Control<NEXT_BUTTON, TextButtonClass>().Enable();
        }

        RulePageIndex--;
        Load_Current_Rules_Page();
        return true;
    }

    bool Load_Next_Rules_Page()
    {
        if (RulePageIndex >= RulePageCount - 1) {
            return false;
        }

        if (RulePageIndex + 1 >= RulePageCount - 1) {
            Get_Control<NEXT_BUTTON, TextButtonClass>().Disable();
        } else {
            Get_Control<PREVIOUS_BUTTON, TextButtonClass>().Enable();
        }

        RulePageIndex++;
        Load_Current_Rules_Page();
        return true;
    }

    void On_Help_Click(const RulesEditorControls& control) const
    {
        auto rule_index = control < LEFT_RULE_EDIT_BUTTON
            ? control - LEFT_RULE_HELP_CONTROL
            : 7 + (control - RIGHT_RULE_HELP_CONTROL);

        rule_index += RulePageIndex * RulesPerPage;

        auto& section = Get_Active_Rule_Sections().Get_Section(ActiveRuleSectionName);
        const auto name = section.Rule_Names()[rule_index];

        WWMessageBox().Process(
            section.Try_Get_Rule_Comment(name).value_or("No help available").c_str()
        );
    }

    void On_Edit_Click(const RulesEditorControls& control)
    {
        auto rule_index = control < RIGHT_PANEL
            ? control - LEFT_RULE_EDIT_BUTTON
            : 7 + (control - RIGHT_RULE_EDIT_BUTTON);

        rule_index += RulePageIndex * RulesPerPage;

        auto& section = Get_Active_Rule_Sections().Get_Section(ActiveRuleSectionName);

        const std::string rule_name = section.Rule_Names()[rule_index].data();

        auto update_to_save = false;
        const auto factor = (SeenBuff.Get_Width() == 320) ? 1 : 2;

        if (TdTypeConverter::Rule_Requires_Csv_Converter(section, rule_name)) {
            ConverterCsvRuleEditorDialog csv_dialog(section, rule_name);
            csv_dialog.Init(
                Try_Get_Resolution_Mode_Width().value_or(SeenBuff.Get_Width()),
                Try_Get_Resolution_Mode_Height().value_or(SeenBuff.Get_Height()),
                factor
            );
            update_to_save = csv_dialog.Present();
        } else {
            ConverterRuleEditorDialog rule_dialog(section, rule_name);
            rule_dialog.Init(
                Try_Get_Resolution_Mode_Width().value_or(SeenBuff.Get_Width()),
                Try_Get_Resolution_Mode_Height().value_or(SeenBuff.Get_Height()),
                factor
            );
            update_to_save = rule_dialog.Present();
        }

        if (update_to_save) {
            Update_Ini_File();
            Load_Current_Rules_Page();
        }
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

    void Set_Active_Rule_Section(const std::string_view section)
    {
        ActiveRuleSectionName = section;

        const auto& active_rule_section = Get_Active_Rule_Sections().Get_Section(section);
        const auto rule_count = active_rule_section.Rule_Names().size();

        // init pagination
        RulePageIndex = 0;
        RulePageCount = static_cast<int>(
            ceil(static_cast<int>(rule_count) / static_cast<double>(RulesPerPage))
        );

        // show/hide pagination controls
        Get_Control<PREVIOUS_BUTTON, TextButtonClass>().Disable();

        if (rule_count <= RulesPerPage) {
            Get_Control<NEXT_BUTTON, TextButtonClass>().Disable();
        } else {
            Get_Control<NEXT_BUTTON, TextButtonClass>().Enable();
        }

        // load first page data
        Load_Current_Rules_Page();
    }

    void Set_Active_Rule_Sections(
        const RuleSections& sections,
        const std::optional<std::string_view>& type_name = std::nullopt
    )
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
        static const auto is_dos = Settings.Video.DOSMode || Is_DOS_Files();
        static const auto previous_str = std::format("{} {}", Text_String(TXT_LEFT), (is_dos ? "Previous" : "Previous Page"));
        static const auto next_str = std::format("{} {}", (is_dos ? "Next" : "Next Page"), Text_String(TXT_RIGHT));

        Add_Button(EXIT_BUTTON, "Exit");

        Add_Button(PREVIOUS_BUTTON, previous_str.c_str()).Disable();
        Add_Button(NEXT_BUTTON, next_str.c_str());

        Add_Button(LOAD_DEFAULTS_BUTTON, is_dos ? "Defaults" : "Load Defaults");
        Add_Button(SAVE_CHANGES_BUTTON, "Save");
    }

    void Init_Right_Rules_Panel()
    {
        auto edit_button = RIGHT_RULE_EDIT_BUTTON;
        auto help_button = RIGHT_RULE_HELP_CONTROL;

        for (auto control = RIGHT_RULE_VALUE_CONTROL; control < RIGHT_RULE_HELP_CONTROL; ++control) {
            Add_Button(edit_button, "Edit"); // TODO: Locale file string
            Add_Button(help_button, "?"); // TODO: Locale file string

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

            ++edit_button;
            ++help_button;
        }
    }

    void Init_Left_Rules_Panel()
    {
        auto edit_button = LEFT_RULE_EDIT_BUTTON;
        auto help_button = LEFT_RULE_HELP_CONTROL;

        for (auto control = LEFT_RULE_VALUE_CONTROL; control < LEFT_RULE_HELP_CONTROL; ++control) {
            Add_Button(edit_button, "Edit"); // TODO: Locale file string
            Add_Button(help_button, "?"); // TODO: Locale file string

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

            ++edit_button;
            ++help_button;
        }
    }

    void Init_Top_Row()
    {
        Add_Button(SEARCH_BUTTON, "Search").Disable(true); // TODO: implement search

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
        ).Disable(true); // TODO: implement search

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
    std::string ActionSectionsTypeName;
    std::string ActiveRuleSectionName;
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

            case LOAD_DEFAULTS_BUTTON | KN_BUTTON: {
                Load_Defaults();

                Collapse_Visible_Dropdowns(display);
                display = REDRAW_ALL;
                break;
            }

            case SAVE_CHANGES_BUTTON | KN_BUTTON: {
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

        // panel edit buttons
        for (auto control = LEFT_RULE_EDIT_BUTTON; control < PREVIOUS_BUTTON; ++control) {
            if (control == RIGHT_PANEL) {
                control = RIGHT_RULE_EDIT_BUTTON;
            }

            if (input == (control | KN_BUTTON)) {
                On_Edit_Click(control);
                display = REDRAW_ALL;
            }
        }

        // panel help buttons
        for (auto control = LEFT_RULE_HELP_CONTROL; control < RIGHT_RULE_EDIT_BUTTON; ++control) {
            if (control == LEFT_RULE_EDIT_BUTTON) {
                control = RIGHT_RULE_HELP_CONTROL;
            }

            if (input == (control | KN_BUTTON)) {
                On_Help_Click(control);
                display = REDRAW_ALL;
            }
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

        auto& file_dropdown = Get_Control<FILE_DROPDOWN, DropListClass>();
        auto& section_dropdown = Get_Control<SECTION_DROPDOWN, DropListClass>();
        const auto file_was_dropped = file_dropdown.IsDropped;
        const auto section_was_dropped = section_dropdown.IsDropped;

        const auto input = Dialog::Get_Input(display);

        if (file_was_dropped && !file_dropdown.IsDropped) {
            display = REDRAW_ALL;
        } else if (!file_was_dropped && file_dropdown.IsDropped) {
            // BUG: workaround for scrollbars not rendering reliably
            file_dropdown.List.Redraw_Scroll_Bar(true);
        }

        if (section_was_dropped && !section_dropdown.IsDropped) {
            display = REDRAW_ALL;
        } else if (!section_was_dropped && section_dropdown.IsDropped) {
            // BUG: workaround for scrollbars not rendering reliably
            section_dropdown.List.Redraw_Scroll_Bar(true);
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
            [&] (const auto& s, const auto& name, const auto& controls) {
                const auto value_control = controls.value_control;
                auto x = value_control < RIGHT_RULE_VALUE_CONTROL
                    ? Dimensions[LEFT_PANEL].X
                    : Dimensions[RIGHT_PANEL].X;
                auto y = value_control < RIGHT_RULE_VALUE_CONTROL ? left_column_y : right_column_y;

                Fancy_Text_Print(name.data(),
                     x + HorizontalSpacing,
                     y,
                     CC_GREEN,
                     TBLACK,
                     TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

                if (value_control < RIGHT_RULE_VALUE_CONTROL) {
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
        auto help_btn = LEFT_RULE_HELP_CONTROL;
        auto edit_btn = LEFT_RULE_EDIT_BUTTON;

        static const auto is_dos = Settings.Video.DOSMode || Is_DOS_Files();

        for (auto control = LEFT_RULE_VALUE_CONTROL; control < LEFT_RULE_HELP_CONTROL; ++control) {
            left_control_y += VerticalSpacing;

            Dimensions[help_btn] = {
                Dimensions[LEFT_PANEL].X + panel_width - HorizontalSpacing - (ControlWidth / 3),
                left_control_y,
                ControlWidth / 3,
                ControlHeight
            };

            Dimensions[edit_btn] = {
                Dimensions[help_btn].X - HorizontalSpacing - (ControlWidth / 2),
                left_control_y,
                is_dos ? ControlWidth : ControlWidth / 2,
                ControlHeight
            };

            left_control_y += (10 * Factor);

            Dimensions[control] = {
                Dimensions[LEFT_PANEL].X + HorizontalSpacing,
                left_control_y,
                panel_width - (HorizontalSpacing * 2),
                ControlHeight
            };

            left_control_y += ControlHeight;
            ++help_btn;
            ++edit_btn;
        }

        // right panel
        Dimensions[RIGHT_PANEL] = {
            ControlsX + panel_width + HorizontalSpacing,
            MiddleRowY,
            panel_width,
            panel_height
        };

        auto right_control_y = Dimensions[RIGHT_PANEL].Y;
        edit_btn = RIGHT_RULE_EDIT_BUTTON;
        help_btn = RIGHT_RULE_HELP_CONTROL;

        for (auto control = RIGHT_RULE_VALUE_CONTROL; control < RIGHT_RULE_HELP_CONTROL; ++control) {
            right_control_y += VerticalSpacing;

            Dimensions[help_btn] = {
                Dimensions[RIGHT_PANEL].X + panel_width - HorizontalSpacing - (ControlWidth / 3),
                right_control_y,
                ControlWidth / 3,
                ControlHeight
            };

            Dimensions[edit_btn] = {
                Dimensions[help_btn].X - HorizontalSpacing - (ControlWidth / 2),
                right_control_y,
                is_dos ? ControlWidth : ControlWidth / 2,
                ControlHeight
            };

            right_control_y += (10 * Factor);

            Dimensions[control] = {
                Dimensions[RIGHT_PANEL].X + HorizontalSpacing,
                right_control_y,
                panel_width - (HorizontalSpacing * 2),
                ControlHeight
            };

            right_control_y += ControlHeight;
            ++help_btn;
            ++edit_btn;
        }

        // bottom row
        Dimensions[EXIT_BUTTON] = {
            ControlsX,
            BottomRowY,
            static_cast<int>(nearbyint(ControlWidth * 0.8)),
            ControlHeight
        };

        Dimensions[NEXT_BUTTON] = {
            Dimensions[RIGHT_PANEL].X,
            BottomRowY,
            static_cast<int>(nearbyint(ControlWidth * 1.25)),
            ControlHeight
        };

        static auto scale = is_dos ? 1.25 : 1.4;

        Dimensions[PREVIOUS_BUTTON] = {
            Dimensions[NEXT_BUTTON].X - static_cast<int>(nearbyint(ControlWidth * scale)) - HorizontalSpacing + 1,
            BottomRowY,
            static_cast<int>(nearbyint(ControlWidth * scale)),
            ControlHeight
        };

        Dimensions[SAVE_CHANGES_BUTTON] = {
            X + Width - static_cast<int>(nearbyint(ControlWidth * 0.8)) - MarginWidth + 1,
            BottomRowY,
            static_cast<int>(nearbyint(ControlWidth * 0.8)),
            ControlHeight
        };

        Dimensions[LOAD_DEFAULTS_BUTTON] = {
            Dimensions[SAVE_CHANGES_BUTTON].X - HorizontalSpacing - static_cast<int>(nearbyint(ControlWidth * 1.2)),
            BottomRowY,
            static_cast<int>(nearbyint(ControlWidth * 1.2)),
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

    Load_Title_Screen(TitlePicture, &HidPage, Palette);
    Blit_Hid_Page_To_Seen_Buff();
    Set_Palette(Palette);

    while (Get_Mouse_State() > 0)
        Show_Mouse();

    const auto result = dialog.Present();

    /*------------------------------------------------------------------------
    Restore screen
    ------------------------------------------------------------------------*/
    Hide_Mouse();
    Load_Title_Screen(TitlePicture, &HidPage, Palette);
    Blit_Hid_Page_To_Seen_Buff();
    Show_Mouse();

    return result;
}
