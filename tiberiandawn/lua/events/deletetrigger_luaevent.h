#pragma once 

#include "../../../common/lua/luaevent.h"

class DeleteTriggerLuaEvent : public LuaEvent
{
public:
    DeleteTriggerLuaEvent(std::string name);

    void Execute() const override;

private:
    std::string Name;
};
