#pragma once

#include <string>

#include "../common/lua/luaapi.h"
#include "../common/lua/luaarguments.h"
#include "../common/lua/luaengine.h"
#include "../common/lua/luatablebuilder.h"

#include "../externs.h"
#include "../defines.h"
#include "../teamtype.h"
#include "../trigger.h"
#include "../type.h"

#include "td_luaapi.h"

class ScenarioLuaApi: public TiberianDawnLuaApi
{
public:
    ScenarioLuaApi(
        std::string scenario_name,
        std::string scenario_type,
        std::string scenario_faction,
        std::string scenario_house
    ) : TiberianDawnLuaApi("Scenario", true)
    {
        Scenario_Name = scenario_name;
        Scenario_Type = scenario_type;
        Scenario_Faction = scenario_faction;
        Scenario_House = scenario_house;
    }

    virtual void Register_Consts(LuaEngine& engine) const override
    {
        With_Api_Namespace(engine, [&](auto& n) {
            n.addConstant("name", Scenario_Name)
                .addConstant("type", Scenario_Type)
                .addConstant("faction", Scenario_Faction)
                .addConstant("house", Scenario_House);
        });
    }

    virtual void Register_Functions(LuaEngine& engine) const override
    {
        With_Api_Namespace(engine, [](auto& n) {
            n.addCFunction("getHouseNames", [](auto L) {
                auto engine = SharedLuaEngine(L);
                
                auto house_name_table = LuaTableBuilder(engine);

                for(auto i = HOUSE_FIRST; i < HOUSE_COUNT; i++) {
                    house_name_table.With_Index_Value(
                        HouseTypeClass::As_Reference(i).IniName
                    );
                }

                return 1;
            })
            .addCFunction("getHouseMoney", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "<number> Scenario.getHouseMoney(<string: name>)");

                arguments.Count_Is(1).First_Argument_Is<std::string>().Assert();

                auto name = arguments.Read_First<std::string>().Unpack();

                auto house = HouseClass::As_Pointer(
                    Parse_House_Name(engine, name)
                );

                engine.Push_Value(
                    house->Available_Money()
                );

                return 1;
            })
            .addCFunction("getTeamTypeNames", [](auto L) {
                auto engine = SharedLuaEngine(L);

                auto trigger_names_table = LuaTableBuilder(engine);
                auto i = 0;

                while (i < Triggers.Count()) {
                    auto trigger_name = Triggers.Ptr(i)->Get_Name();

                    trigger_names_table.With_Index_Value(trigger_name);
                    i++;
                }

                return 1;
            })
            .addCFunction("getTeamType", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "<string> Scenario.getTeamType(<string: name>...)");

                arguments.Count_Is(1)
                    .First_Argument_Is<std::string>()
                    .Assert();
    
                auto team_type_name = arguments.Read_First<std::string>().Unpack();

                auto team_type = TeamTypeClass::As_Pointer(team_type_name.data());

                if (team_type == NULL) {
                    engine.Raise_Error_Format("TeamType not found: {}", team_type_name);
                }

                char team_type_definition[500];

                team_type->Write_INI_String(team_type_definition);

                engine.Push_Value(team_type_definition);

                return 1;
            })
            .addCFunction("addTeamType", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Scenario.addTeamType(<string: name>, <string: definitionCsv>)");

                arguments.Count_Is(2)
                    .First_Argument_Is<std::string>()
                    .First_Argument_Is<std::string>()
                    .Assert();

                auto name = arguments.Read_First<std::string>().Unpack();
                auto definition = arguments.Read_Next<std::string>().Unpack();

                // TODO: implement value validators in LuaArguments
                if (name.length() < 1) {
                    engine.Raise_Error_Format("Team type name was empty");
                } else if (name.length() > 8) {
                    engine.Raise_Error_Format(
                        "Team type name '{}' too long, should be at most 8 characters long. Name: {}",
                        name
                    );
                }

                if (definition.length() < 1) {
                    engine.Raise_Error_Format("Team type CSV definition was empty");
                } else if (definition.length() > 127) {
                    engine.Raise_Error_Format(
                        "Team type definition too long, should be at most 127 characters long. Definition: {}",
                        definition
                    );
                }

                CNC_LOGGER_DEBUG(
                    "Loading team type '{}' from Lua call, CSV definition: {}",
                    name,
                    definition
                );

                auto team_type = new TeamTypeClass();

                team_type->Fill_In(name.data(), definition.data());

                return 0;
            })
            .addCFunction("getTriggerNames", [](auto L) {
                auto engine = SharedLuaEngine(L);

                auto trigger_names_table = LuaTableBuilder(engine);
                auto i = 0;

                while (i < Triggers.Count()) {
                    auto trigger_name = Triggers.Ptr(i)->Get_Name();

                    trigger_names_table.With_Index_Value(trigger_name);
                    i++;
                }

                return 1;
            })
            .addCFunction("getTrigger", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "<string> Scenario.getTeamType(<string: name>...)");

                arguments.Count_Is(1)
                    .First_Argument_Is<std::string>()
                    .Assert();
    
                auto trigger_name = arguments.Read_First<std::string>().Unpack();

                auto trigger = TriggerClass::As_Pointer(trigger_name.c_str());

                if (trigger == NULL) {
                    engine.Raise_Error_Format("Trigger not found: {}", trigger_name);
                }

                char trigger_definition[128];

                trigger->Write_INI_String(trigger_definition);

                engine.Push_Value(trigger_definition);

                return 1;
            })
            .addCFunction("addTrigger", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Scenario.addTrigger(<string: name>, <string: definitionCsv>)");

                arguments.Count_Is(2)
                    .First_Argument_Is<std::string>()
                    .First_Argument_Is<std::string>()
                    .Assert();

                auto trigger_name = arguments.Read_First<std::string>().Unpack();
                auto trigger_definition = arguments.Read_Next<std::string>().Unpack();

                // TODO: implement value validators in LuaArguments
                if (trigger_name.length() < 1) {
                    engine.Raise_Error_Format("Trigger name was empty");
                } else if (trigger_name.length() > 4) {
                    engine.Raise_Error_Format(
                        "Trigger name '{}' too long, should be at most 4 characters long. Name: {}",
                        trigger_name
                    );
                }

                if (trigger_name.length() < 1) {
                    engine.Raise_Error_Format("Trigger CSV definition was empty");
                } else if (trigger_definition.length() > 127) {
                    engine.Raise_Error_Format(
                        "Trigger definition too long, should be at most 127 characters long. Definition: {}",
                        trigger_definition
                    );
                }

                CNC_LOGGER_DEBUG(
                    "Loading scenario trigger '{}' from Lua call, CSV definition: {}",
                    trigger_name,
                    trigger_definition
                );

                auto trigger = new TriggerClass();

                trigger->Fill_In(
                    trigger_name.c_str(),
                    trigger_definition.c_str()
                );

                trigger->Load();

                return 0;
            })
            .addCFunction("deleteTriggerIfExists", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "<bool> Scenario.deleteTriggerIfExists(<string: name>)");

                arguments.Count_Is(1).First_Argument_Is<std::string>().Assert();

                auto name = arguments.Read_First<std::string>().Unpack();
                auto trigger = TriggerClass::As_Pointer(name.c_str());
                auto trigger_exists = trigger != NULL;

                if (trigger_exists) {
                    CNC_LOGGER_DEBUG("Removing scenario trigger: {}", name);

                    // TODO: consider lua event for thread safety
                    trigger->Remove();
                    delete trigger;
                }

                engine.Push_Value(trigger_exists);

                return 1;
            });
        });
    }

protected:
    virtual const char* Get_Cpp_Source() const override
    {
        return __FILE__;
    }

private:
    static HousesType Parse_House_Name(const LuaEngine& engine, const std::string& name)
    {
        auto houseType = HouseTypeClass::From_Name(name.c_str());

        if (houseType == HOUSE_NONE) {
            engine.Raise_Error_Format(
                "Failed to parse house name from string: {}",
                name
            );
        }

        return houseType;
    }

    std::string Scenario_Name;
    std::string Scenario_Type;
    std::string Scenario_Faction;
    std::string Scenario_House;
};
