#pragma once 

#include "../../../common/lua/luaevent.h"

class AddTeamLuaEvent : public LuaEvent
{
public:
    AddTeamLuaEvent(std::string name, std::string definition);

    void Execute() const override;

private:
    std::string Name;
    std::string Definition;
};
