#pragma once 

#include "../common/lua/luaevent.h"

#include "../msgbox.h"

class PopupLuaEvent : public LuaEvent {
public:
    PopupLuaEvent(std::string message) : LuaEvent("PopupLuaEvent", message) {}

    virtual void Execute() const override {
        LuaEvent::Execute();

        auto message = std::get<std::string>(Data);

        WWMessageBox().Process(message.data());
    }
};
