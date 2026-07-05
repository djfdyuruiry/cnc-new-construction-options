#pragma once

#include <optional>
#include <map>
#include <memory>
#include <type_traits>

#include "common/framelimit.h"
#include "common/gadget.h"
#include "common/logger.h"

#include "function.h"
#include "drop.h"

struct DialogControlDimension
{
    int X = 0;
    int Y = 0;
    int W = 0;
    int H = 0;
};

template<typename T> concept EnumerationType = std::is_enum_v<T>;

template<EnumerationType T>
class Dialog
{
protected:
    /*........................................................................
    Redraw values: in order from "top" to "bottom" layer of the dialog
    ........................................................................*/
    typedef enum
    {
        REDRAW_NONE = 0,
        REDRAW_FOREGROUND, // dynamic text, custom control-like objects
        REDRAW_CONTROLS, // controls
        REDRAW_BACKGROUND, // static text, decorations
        REDRAW_ALL = REDRAW_BACKGROUND
    } DialogRedrawType;

    static inline const auto& Logger = CncLogger::For(Dialog);

    // base dimensions (not scaled)

    int BaseWidth;
    int BaseHeight;
    int BaseMarginWidth;
    int BaseMarginHeight;

    // display scale
    int Factor;

    // dimensions (scaled)

    int X;
    int Y;
    int Width;
    int Height;
    int CenterX;
    int CenterY;

    int TextHeight;
    int MarginWidth;
    int MarginHeight;

    // button shapes

    void const* UpButtonShape;
    void const* DownButtonShape;

    // controls

    int CaptionTextId;
    std::optional<std::string> CaptionText;
    std::map<T, DialogControlDimension> Dimensions;
    std::map<T, std::unique_ptr<char[]>> Text;
    std::map<T, std::unique_ptr<GadgetClass>> Controls;
    GadgetClass* CommandChain;

    bool Is_Mouse_Over_Rectangle(const int start_x, const int start_y, const int end_x, const int end_y)
    {
        return Keyboard->MouseQX >= start_x
            && Keyboard->MouseQX <= end_x
            && Keyboard->MouseQY >= start_y
            && Keyboard->MouseQY <= end_y;
    }

    bool Is_Mouse_Outside_Control_Dimensions(const ControlClass& control)
    {
        return (Keyboard->MouseQX < control.X || Keyboard->MouseQX > control.X + control.Width)
            && (Keyboard->MouseQY < control.Y || Keyboard->MouseQY > control.Y + control.Height);
    }

    bool Is_Mouse_Outside_Dropdown_Dimensions(const DropListClass& control)
    {
        return Is_Mouse_Outside_Control_Dimensions(control)
            && Is_Mouse_Outside_Control_Dimensions(control.List)
            && Is_Mouse_Outside_Control_Dimensions(control.DropButton);
    }

    template<std::derived_from<GadgetClass> U>
    U& Get_Control(const T type)
    {
        return *reinterpret_cast<U*>(Controls[type].get());
    }

    template<T type, std::derived_from<GadgetClass> U>
    U& Get_Control()
    {
        return Get_Control<U>(type);
    }

    template<std::derived_from<GadgetClass> U, typename... Args>
    U& Add_Control(const T type, Args&&... args)
    {
        Controls[type] = std::make_unique<U>(type, std::forward<Args>(args)...);

        auto& control = Get_Control<U>(type);

        // add control to command chain so we can capture input
        if (CommandChain == nullptr) {
            if constexpr (std::is_same_v<U, DropListClass>) {
                CNC_LOGGER_FATAL(
                    "Attempted to use DropListClass as first control in command chain, "
                    "this results in a broken dropdown."
                );
            }

            // first element in chain
            CommandChain = &control;
        } else {
            // nth element in chain
            control.Add_Tail(*CommandChain);
        }

        return control;
    }

    template<T type, std::derived_from<GadgetClass> U, typename... Args>
    U& Add_Control(Args&&... args)
    {
        return Add_Control<U>(type, std::forward<Args>(args)...);
    }

    TextButtonClass& Add_Button(
        T control,
        std::variant<const char*, int> text,
        const TextPrintType font_style = TPF_6PT_GRAD | TPF_NOSHADOW
    )
    {
        if (std::holds_alternative<const char*>(text)) {
            // hard coded label
            return Add_Control<TextButtonClass>(
                control,
                std::get<const char*>(text),
                font_style,
                Dimensions[control].X,
                Dimensions[control].Y,
                Dimensions[control].W,
                Dimensions[control].H
            );
        }

        return Add_Control<TextButtonClass>(
            control,
            std::get<int>(text),
            font_style,
            Dimensions[control].X,
            Dimensions[control].Y,
            Dimensions[control].W,
            Dimensions[control].H
        );
    }

    virtual std::optional<bool> On_Input(DialogRedrawType& display, KeyNumType& input) = 0;

    virtual KeyNumType Get_Input(DialogRedrawType& display)
    {
        return CommandChain != nullptr ? CommandChain->Input() : KN_NONE;
    }

    virtual void Render_Foreground(DialogRedrawType& display) {}

    virtual void Render_Background(DialogRedrawType& display)
    {
        Dialog_Box(X, Y, Width, Height);

        // init font variables
        Fancy_Text_Print(
            TXT_NONE, 0, 0, TBLACK, TBLACK, TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW
        );

        if (CaptionText.has_value()) {
            // OPTION_DIALOG param will draw the decoration in the corners of the dialog
            Draw_Caption(CaptionText->c_str(), OPTION_DIALOG, X, Y, Width);
        } else {
            // the text id param will determine the dialog decorations (if any, see: OptionControlType)
            Draw_Caption(CaptionTextId, X, Y, Width);
        }
    }

    void Render(DialogRedrawType& display)
    {
        if (!display) {
            return;
        }

        Hide_Mouse();

        if (display >= REDRAW_BACKGROUND) {
            Render_Background(display);
        }

        if (display >= REDRAW_FOREGROUND) {
            Render_Foreground(display);
        }

        if (display >= REDRAW_CONTROLS && CommandChain != nullptr) {
            CommandChain->Flag_List_To_Redraw();
        }

        Show_Mouse();
        display = REDRAW_NONE;
    }

    virtual void Init_UI_State() = 0;
    virtual void Init_Data() = 0;

    virtual void Init_Controls()
    {
        Text.clear();
        Controls.clear();
        CommandChain = nullptr;
    }

    virtual void Init_Shapes()
    {
        if (InMainLoop || Factor == 1) {
            UpButtonShape = UpButtonShape == nullptr ? Hires_Retrieve("BTN-UP.SHP") : UpButtonShape;
            DownButtonShape = DownButtonShape == nullptr ? Hires_Retrieve("BTN-DN.SHP") : DownButtonShape;
        } else {
            UpButtonShape = UpButtonShape == nullptr ? Hires_Retrieve("BTN-UP2.SHP") : UpButtonShape;
            DownButtonShape = DownButtonShape == nullptr ? Hires_Retrieve("BTN-DN2.SHP") : DownButtonShape;
        }
    }

    virtual void Init_Dimensions(const int screen_width, const int screen_height, const int factor)
    {
        Factor = factor;

        Width = BaseWidth * Factor;
        Height = BaseHeight * Factor;
        X = (screen_width - Width) / 2;
        Y = (screen_height - Height) / 2;
        CenterX = X + (Width / 2);
        CenterY = Y + (Height / 2);

        TextHeight = 6 * Factor + 1; // ht of 6-pt text
        MarginWidth = BaseMarginWidth * Factor;
        MarginHeight = BaseMarginHeight * Factor;

        Dimensions.clear();
    }

    Dialog(
        const int width,
        const int height,
        const int margin_width = 0,
        const int margin_height = 0
    )
        : BaseWidth(width)
        , BaseHeight(height)
        , BaseMarginWidth(margin_width)
        , BaseMarginHeight(margin_height)
        , Factor(0)
        , X(0)
        , Y(0)
        , Width(0)
        , Height(0)
        , CenterX(0)
        , TextHeight(0)
        , MarginWidth(0)
        , MarginHeight(0)
        , UpButtonShape(nullptr)
        , DownButtonShape(nullptr)
        , CaptionTextId(TXT_NONE)
        , CaptionText(std::nullopt)
        , Dimensions()
        , Text()
        , Controls()
        , CommandChain(nullptr)
    {
    }
    virtual ~Dialog() = default;

public:
    virtual void Init(const int screen_width, const int screen_height, const int factor)
    {
        Init_Dimensions(screen_width, screen_height, factor);
        Init_Shapes();
        Init_Controls();
        Init_Data();
        Init_UI_State();
    }

    virtual bool Present()
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

            auto result = On_Input(display, input);

            if (result.has_value()) {
                return *result;
            }

            Frame_Limiter();
        }
    }
};
