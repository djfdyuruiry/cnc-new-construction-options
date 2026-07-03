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
    LEFT_RULE_VALUE_CONTROL, // 6 controls
    LEFT_RULE_HELP_CONTROL = 211, // 6 controls
    RIGHT_PANEL = 217, // virtual panel used for drawing
    RIGHT_RULE_VALUE_CONTROL,  // 6 controls
    RIGHT_RULE_HELP_CONTROL = 223, // 6 controls
    PREVIOUS_BUTTON = 229,
    NEXT_BUTTON,
    EXIT_BUTTON
} RulesEditorControls;

class RulesEditorDialog : public Dialog<RulesEditorControls>
{
    static constexpr auto DropdownTextLength = 25;

    void Init_Bottom_Row()
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

    int ControlWidth;
    int ControlHeight;
    int DropdownItemWidth;
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

        // left panel border
        LogicPage->Draw_Rect(
            Dimensions[LEFT_PANEL].X,
            Dimensions[LEFT_PANEL].Y,
            Dimensions[LEFT_PANEL].X + Dimensions[LEFT_PANEL].W,
            Dimensions[LEFT_PANEL].Y + Dimensions[LEFT_PANEL].H,
            LTGRAY
        );

        // right panel border
        LogicPage->Draw_Rect(
            Dimensions[RIGHT_PANEL].X,
            Dimensions[RIGHT_PANEL].Y,
            Dimensions[RIGHT_PANEL].X + Dimensions[RIGHT_PANEL].W,
            Dimensions[RIGHT_PANEL].Y + Dimensions[RIGHT_PANEL].H,
            LTGRAY
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
        Init_Bottom_Row();
    }

    void Init_Dimensions(const int screen_width, const int screen_height, const int factor) override
    {
        Dialog::Init_Dimensions(screen_width, screen_height, factor);

        ControlWidth = 40 * Factor;
        ControlHeight = 9 * Factor;
        DropdownItemWidth = 8 * Factor;
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
            5 * DropdownItemWidth // 5 visible items (excluding selected value)
        };

        Dimensions[SECTION_DROPDOWN] = {
            ControlsX + Dimensions[FILE_DROPDOWN].W + (HorizontalSpacing * 4),
            TopRowY,
            static_cast<int>(nearbyint(ControlWidth * 1.75)),
            10 * DropdownItemWidth // 10 visible items (excluding selected value)
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

        // bottom row
        Dimensions[EXIT_BUTTON] = {ControlsX, BottomRowY, ControlWidth, ControlHeight};

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

        // panels (middle row)
        const auto panel_width = ((Width - (MarginWidth * 2)) / 2) - VerticalSpacing;
        const auto panel_height = Dimensions[NEXT_BUTTON].Y - MiddleRowY - VerticalSpacing;

        Dimensions[LEFT_PANEL] = {
            ControlsX,
            MiddleRowY,
            panel_width,
            panel_height
        };

        Dimensions[RIGHT_PANEL] = {
            X + Width - MarginWidth - panel_width,
            MiddleRowY,
            panel_width,
            panel_height
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
