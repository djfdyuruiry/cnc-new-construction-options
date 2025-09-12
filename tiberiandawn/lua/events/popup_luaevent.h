#pragma once 

#include "../../../common/lua/luaevent.h"

#include "../../msgbox.h"

class PopupLuaEvent : public LuaEvent
{
public:
    PopupLuaEvent(std::string message) : LuaEvent("Popup")
    {
        Message = message;
    }

    virtual void Execute() const override
    {
        LuaEvent::Execute();

        WWMessageBox().Process(Message.c_str());
    }

private:
    std::string Message;
};
