#pragma once 

#include "../../../common/lua/luaevent.h"

class AddMessageLuaEvent : public LuaEvent
{
public:
    AddMessageLuaEvent(std::string message);

    void Execute() const override;

private:
    // event data
    std::string Message;
};
