#include "function.h"

#include "popup_luaevent.h"

PopupLuaEvent::PopupLuaEvent(std::string message): LuaEvent("Popup")
{
    Message = std::move(message);
}

void PopupLuaEvent::Execute() const
{
    LuaEvent::Execute();

    WWMessageBox().Process(Message.c_str());
}
