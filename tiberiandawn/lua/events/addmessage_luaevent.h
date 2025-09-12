#pragma once 

#include "../../../common/lua/luaevent.h"

#include "../../externs.h"

class AddMessageLuaEvent : public LuaEvent
{
public:
    AddMessageLuaEvent(
        std::string message
    ) : LuaEvent("AddMessage")
    {
        Message = message;
    }

    virtual void Execute() const override
    {
        LuaEvent::Execute();

        // BUG: char* data() call to Message doesn't resolve even
        //      though it's not a const (cast required on const method)
        std::string message = Message;

        // ripped off from netdlg.cpp
        Messages.Add_Message(
            message.data(),
            CC_GREEN,
            TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW,
            600,
            0,
            0
        );
        Map.Flag_To_Redraw(false);
    }

private:
    // event data
    std::string Message;
};