#pragma once

#include "function.h"
#include "list.h"

#ifdef NEWMENU

class EListClass : public ListClass
{
    // data might be embedded in entry list strings, so we use this offset to know where actual text data begins
    int TextOffset;

public:
    EListClass(
        const int id,
        const int x,
        const int y,
        const int w,
        const int h,
        const TextPrintType flags,
        void const* up,
        void const* down,
        const int text_offset = 0
    ) : ListClass(id, x, y, w, h, flags, up, down)
    {
        TextOffset = text_offset;
    }

    void Clear() {
        List.Clear();

        if (IsScrollActive) {
            Remove_Scroll_Bar();
        }

        Set_Selected_Index(0);
    }

protected:
    void Draw_Entry(const int index, const int x, const int y, const int width, const int selected) override
    {
        if (TextFlags & TPF_6PT_GRAD) {
            auto flags = TextFlags;

            if (selected) {
                flags = flags | TPF_BRIGHT_COLOR;
                LogicPage->Fill_Rect(x, y, x + width - 1, y + LineHeight - 1, CC_GREEN_SHADOW);
            } else {
                if (!(flags & TPF_USE_GRAD_PAL)) {
                    flags = flags | TPF_MEDIUM_COLOR;
                }
            }

            Conquer_Clip_Text_Print(List[index] + TextOffset, x, y, CC_GREEN, TBLACK, flags, width, Tabs);
        } else {
            Conquer_Clip_Text_Print(
                List[index] + TextOffset, x, y, (selected ? BLUE : WHITE), TBLACK, TextFlags, width, Tabs
            );
        }
    }
};

#endif
