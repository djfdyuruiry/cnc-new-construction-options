#include "function.h"

#include "dialog.h"
#include "drop.h"

typedef enum
{
    FILE_DROPDOWN = 200,
    SECTION_DROPDOWN,
    SEARCH_TEXTBOX,
    SEARCH_BUTTON,
    LEFT_RULE_VALUE_CONTROL, // 12 controls
    LEFT_RULE_HELP_CONTROL = 210,
    RIGHT_RULE_VALUE_CONTROL = 216,  // 12 controls
    RIGHT_RULE_HELP_CONTROL = 222,
    PREVIOUS_BUTTON = 228,
    NEXT_BUTTON,
    EXIT_BUTTON
} RulesEditorControls;

class RulesEditorDialog : public Dialog<RulesEditorControls>
{
    static constexpr auto DropdownTextLength = 25;

    void Init_Bottom_row()
    {
        Add_Button(PREVIOUS_BUTTON, "Previous");
        Add_Button(NEXT_BUTTON, "Next");
        Add_Button(EXIT_BUTTON, "Exit");
    }

    void Init_Right_Rules_Panel()
    {

    }

    void Init_Left_Rules_Panel()
    {

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

protected:
    std::optional<bool> On_Input(DialogRedrawType& display, KeyNumType& input) override
    {
        switch (input) {
            case KN_RETURN:
            case EXIT_BUTTON | KN_BUTTON: {
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

        const auto panel_width = (Width - (MarginWidth * 2)) / 2;
        const auto control_height = 9 * Factor;

        LogicPage->Draw_Rect(
            X + MarginWidth,
            Dimensions[FILE_DROPDOWN].Y + control_height + (2 * Factor),
            X + Width - MarginWidth - panel_width - 1 - 2,
            Dimensions[NEXT_BUTTON].Y - (2 * Factor),
            LTGREEN
        );

        LogicPage->Draw_Rect(
            X + Width - MarginWidth - panel_width + 1,
            Dimensions[FILE_DROPDOWN].Y + control_height + (2 * Factor),
            X + Width - MarginWidth - 2,
            Dimensions[NEXT_BUTTON].Y - (2 * Factor),
            LTGREEN
        );
    }

    void Init_UI_State() override
    {
    }

    void Init_Data() override
    {
        auto& file_dropdown = Get_Control<FILE_DROPDOWN, DropListClass>();
        auto& section_dropdown = Get_Control<SECTION_DROPDOWN, DropListClass>();

        file_dropdown.Add_Item("rules.ini");
        file_dropdown.Add_Item("infantry.ini");
        file_dropdown.Add_Item("unit.ini");
        file_dropdown.Add_Item("aircraft.ini");
        file_dropdown.Add_Item("building.ini");
        file_dropdown.Set_Selected_Index(0);
        file_dropdown.Set_Read_Only(true);

        for (const auto& section_name : Rule.Get_Rule_Sections().Section_Names()) {
            section_dropdown.Add_Item(section_name.data());
        }

        section_dropdown.Set_Selected_Index(0);
        section_dropdown.Set_Read_Only(true);
    }

    void Init_Controls() override
    {
        Dialog::Init_Controls();

        Init_Top_Row();
        Init_Left_Rules_Panel();
        Init_Right_Rules_Panel();
        Init_Bottom_row();
    }

    void Init_Dimensions(const int screen_width, const int screen_height, const int factor) override
    {
        Dialog::Init_Dimensions(screen_width, screen_height, factor);

        const auto control_width = 40 * Factor;
        const auto control_height = 9 * Factor;
        const auto dropdown_item_width = 8 * Factor;
        const auto horizontal_spacing = 3 * Factor;
        const auto vertical_spacing = 2 * Factor;

        const auto top_row_x = X + MarginWidth;
        const auto top_row_y = Y + MarginHeight + (13 * Factor);

        Dimensions[FILE_DROPDOWN]= {
            top_row_x,
            top_row_y,
            static_cast<int>(nearbyint(control_width * 1.2)),
            5 * dropdown_item_width // 5 visible items (excluding selected value)
        };

        Dimensions[SECTION_DROPDOWN] = {
            top_row_x + Dimensions[FILE_DROPDOWN].W + (horizontal_spacing * 4),
            top_row_y,
            static_cast<int>(nearbyint(control_width * 1.75)),
            10 * dropdown_item_width // 10 visible items (excluding selected value)
        };

        Dimensions[SEARCH_BUTTON] = {
            X + Width - control_width - MarginWidth,
            top_row_y,
            control_width,
            control_height
        };

        Dimensions[SEARCH_TEXTBOX].X = Dimensions[SEARCH_BUTTON].X - (control_width * 2) - horizontal_spacing,
        Dimensions[SEARCH_TEXTBOX].Y = top_row_y;
        Dimensions[SEARCH_TEXTBOX].W = control_width * 2;
        Dimensions[SEARCH_TEXTBOX].H = control_height;

        Dimensions[EXIT_BUTTON].X = X + Width - control_width - MarginWidth;
        Dimensions[EXIT_BUTTON].Y = Y + Height - control_height - MarginHeight;
        Dimensions[EXIT_BUTTON].W = control_width;
        Dimensions[EXIT_BUTTON].H = control_height;

        Dimensions[NEXT_BUTTON] = {
            Dimensions[EXIT_BUTTON].X,
            Dimensions[EXIT_BUTTON].Y - control_height - vertical_spacing,
            control_width,
            control_height
        };

        Dimensions[PREVIOUS_BUTTON] = {
            Dimensions[NEXT_BUTTON].X - control_width - horizontal_spacing,
            Dimensions[NEXT_BUTTON].Y,
            control_width,
            control_height
        };
    }

public:
    RulesEditorDialog() : Dialog(300, 195, 5, 5)
    {
        CaptionText = "Rules Editor";
    }

    ~RulesEditorDialog() override
    {
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
