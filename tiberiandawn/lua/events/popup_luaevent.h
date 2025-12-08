#pragma once 

#include "common/lua/luaevent.h"

class PopupLuaEvent : public LuaEvent
{
public:
    PopupLuaEvent(std::string message) ;

    void Execute() const override;

private:
    std::string Message;
};
