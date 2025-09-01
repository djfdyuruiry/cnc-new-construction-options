#pragma once 

#include "../common/lua/luaevent.h"

#include "../msgbox.h"

class PopupEvent : public LuaEvent {
public:
    PopupEvent(const std::string message) : LuaEvent(message) {}

    virtual void Execute() const {
        auto message = std::get<std::string>(Data);

        WWMessageBox().Process(message.data());
    }
};
