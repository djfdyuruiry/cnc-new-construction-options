#pragma once 

#include "common/lua/luaevent.h"

class AddMessageLuaEvent : public LuaEvent
{
public:
    AddMessageLuaEvent(std::string message, ColorType colour = GREEN);

    void Execute() const override;

private:
    // event data
    std::string Message;
    ColorType Colour;
};
