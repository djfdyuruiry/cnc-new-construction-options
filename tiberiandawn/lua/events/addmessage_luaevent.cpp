#include "function.h"

#include "addmessage_luaevent.h"

AddMessageLuaEvent::AddMessageLuaEvent(std::string message): LuaEvent("AddMessage")
{
    Message = std::move(message);
}

void AddMessageLuaEvent::Execute() const
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