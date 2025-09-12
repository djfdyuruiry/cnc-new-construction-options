#pragma once 

#include "../../../common/lua/luaevent.h"

#include "../../teamtype.h"

class AddTeamLuaEvent : public LuaEvent
{
public:
    AddTeamLuaEvent(
        std::string name,
        std::string definition
    ) : LuaEvent("AddTeam")
    {
        Name = name;
        Definition = definition;
    }

    virtual void Execute() const override
    {
        LuaEvent::Execute();

        CNC_LOGGER_DEBUG(
            "Loading team type '{}' from Lua call, CSV definition: {}",
            Name,
            Definition
        );

        auto team_type = new TeamTypeClass();

        team_type->Fill_In(Name.c_str(), Definition.c_str());
    }

private:
    std::string Name;
    std::string Definition;
};
