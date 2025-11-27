#pragma once

#include <string>

#include "../common/lua/luaengine.h"

#include "../defines.h"

#include "td_luaapi.h"

class ScenarioLuaApi: public TiberianDawnLuaApi
{
public:
    ScenarioLuaApi(
        std::string scenario_name,
        std::string scenario_type,
        std::string scenario_faction,
        std::string scenario_house
    ) ;

    void Register_Consts(LuaEngine& engine) const override;

    void Register_Functions(LuaEngine& engine) const override;

protected:
    constexpr const char* Get_Cpp_Source() const override
    {
        return __FILE__;
    }

private:
    static HousesType Parse_House_Name(const LuaEngine& engine, std::string name);

    std::string ScenarioName;
    std::string ScenarioType;
    std::string ScenarioFaction;
    std::string ScenarioHouse;
};
