#pragma once 

#include "common/lua/luaevent.h"

class AddTriggerLuaEvent : public LuaEvent
{
public:
    AddTriggerLuaEvent(std::string name, std::string definition);

    void Execute() const override;

private:
    std::string Name;
    std::string Definition;
};
