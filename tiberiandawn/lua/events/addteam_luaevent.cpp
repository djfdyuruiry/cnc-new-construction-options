#include "function.h"

#include "addteam_luaevent.h"

AddTeamLuaEvent::AddTeamLuaEvent(std::string name, std::string definition): LuaEvent("AddTeam")
{
    Name = std::move(name);
    Definition = std::move(definition);
}

// BUG: Game crashes on parse if definition CSV is not valid
void AddTeamLuaEvent::Execute() const
{
    LuaEvent::Execute();

    CNC_LOGGER_DEBUG(
        "Loading team type '{}' from Lua call, CSV definition: {}",
        Name,
        Definition
    );

    const auto team_type = new TeamTypeClass();

    team_type->Fill_In(Name.c_str(), Definition.c_str());
}
