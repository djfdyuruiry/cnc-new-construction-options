#pragma once 

#include "common/lua/luaevent.h"

class AddMessageLuaEvent : public LuaEvent
{
public:
    AddMessageLuaEvent(std::string message, CCPaletteType colour = CC_GREEN, int timeoutInTicks = 600);

    void Execute() const override;

private:
    // event data
    std::string Message;
    CCPaletteType Colour;
    int TimeoutInTicks;
};
