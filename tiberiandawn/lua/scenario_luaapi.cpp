#include "common/lua/logging_luaapi.h"
#include "common/lua/luaapi.h"
#include "common/lua/luaarguments.h"
#include "common/lua/luatablebuilder.h"

#include "function.h"

#include "events/addteam_luaevent.h"
#include "events/addtrigger_luaevent.h"
#include "events/deletetrigger_luaevent.h"
#include "events/modifyhousemoney_luaevent.h"
#include "scenario_luaapi.h"

ScenarioLuaApi::ScenarioLuaApi(
    std::string scenario_name,
    std::string scenario_type,
    std::string scenario_faction,
    std::string scenario_house,
    const bool was_loaded_from_save
) : TiberianDawnLuaApi("Scenario", true)
{
    ScenarioName = std::move(scenario_name);
    ScenarioType = std::move(scenario_type);
    ScenarioFaction = std::move(scenario_faction);
    ScenarioHouse = std::move(scenario_house);
    WasLoadedFromSave = was_loaded_from_save;
}

void ScenarioLuaApi::Register_Dependencies(LuaEngine& engine) const
{
    engine.Register_Api<LoggingLuaApi>();
}

void ScenarioLuaApi::Register_Consts(LuaEngine& engine) const
{
    With_Api_Namespace(engine, [&](auto& n) {
        n.addConstant("name", ScenarioName)
            .addConstant("type", ScenarioType)
            .addConstant("faction", ScenarioFaction)
            .addConstant("house", ScenarioHouse)
            .addConstant("wasLoadedFromSave", WasLoadedFromSave);
    });
}

void ScenarioLuaApi::Register_Functions(LuaEngine& engine) const
{
    With_Api_Namespace(engine, [](auto& n) {
        n.addCFunction("getHouseNames", [](auto L) {
            const auto engine = SharedLuaEngine(L);

            auto house_name_table = LuaTableBuilder::Push_New_Table(engine);

            for(auto i = HOUSE_FIRST; i < HOUSE_COUNT; ++i) {
                house_name_table.With_Index_Value(
                    HouseTypeClass::As_Reference(i).IniName
                );
            }

            return 1;
        })
        .addCFunction("getHouseMoney", [](auto L) {
            const auto engine = SharedLuaEngine(L);
            auto arguments = LuaArguments(engine, "<number> Scenario.getHouseMoney(<string: name>)");

            arguments.Count_Is(1).First_Argument_Is<std::string>().Assert();

            const auto name = arguments.Read_First<std::string>().Unpack();

            const auto house_type = TdTypeConverter::Assert_Parse_Lua_String<HousesType>(engine, name);
            const auto house = HouseClass::As_Pointer(house_type);

            if (house == nullptr) {
                engine.Raise_Error_Format("Failed to resolve instance for house: {}", house_type);
            }

            engine.Push_Value(
                house->Available_Money()
            );

            return 1;
        })
        .addCFunction("modifyHouseMoney", [](auto L){
            const auto engine = SharedLuaEngine(L);
            auto arguments = LuaArguments(engine, "Scenario.modifyHouseMoney(<string: name>, <number: modifier>)");

            arguments.Count_Is(2)
                .First_Argument_Is<std::string>()
                .Next_Argument_Is<int>()
                .Assert();

            const auto name = arguments.Read_First<std::string>().Unpack();
            const auto money_modifier = arguments.Read_Next<int>().Unpack();

            const auto house = TdTypeConverter::Assert_Parse_Lua_String<HousesType>(engine, name);

            LuaList.Push<ModifyHouseMoneyLuaEvent>(house, money_modifier);

            return 0;
        })
        .addCFunction("getTeamTypeNames", [](auto L) {
            const auto engine = SharedLuaEngine(L);

            auto team_names = LuaTableBuilder::Push_New_Table(engine);

            for (auto i = 0; i < TeamTypes.Count(); ++i) {
                team_names.With_Index_Value(
                    TeamTypes.Ptr(i)->Name()
                );
            }

            return 1;
        })
        .addCFunction("getTeamType", [](auto L) {
            const auto engine = SharedLuaEngine(L);
            auto arguments = LuaArguments(engine, "<string> Scenario.getTeamType(<string: name>...)");

            arguments.Count_Is(1)
                .First_Argument_Is<std::string>()
                .Assert();

            auto team_type_name = arguments.Read_First<std::string>().Unpack();

            const auto team_type = TeamTypeClass::As_Pointer(team_type_name.data());

            if (team_type == nullptr) {
                engine.Raise_Error_Format("TeamType not found: {}", team_type_name);
            }

            char team_type_definition[500];

            team_type->Write_INI_String(team_type_definition);

            engine.Push_Value(team_type_definition);

            return 1;
        })
        .addCFunction("addTeamType", [](auto L) {
            const auto engine = SharedLuaEngine(L);
            auto arguments = LuaArguments(engine, "Scenario.addTeamType(<string: name>, <string: definitionCsv>)");

            arguments.Count_Is(2)
                .First_Argument_Is<std::string>()
                .First_Argument_Is<std::string>()
                .Assert();

            const auto name = arguments.Read_First<std::string>().Unpack();
            const auto definition = arguments.Read_Next<std::string>().Unpack();

            arguments.Assert_String_Parameter_Is_Valid("name", name, 8);
            arguments.Assert_String_Parameter_Is_Valid("definition", definition, 127);

            // TODO: Validate method create, and call before push
            LuaList.Push<AddTeamLuaEvent>(name, definition);

            return 0;
        })
        .addCFunction("getTriggerNames", [](auto L) {
            const auto engine = SharedLuaEngine(L);

            auto trigger_names_table = LuaTableBuilder::Push_New_Table(engine);

            for (auto i = 0; i < Triggers.Count(); ++i) {
                trigger_names_table.With_Index_Value(
                    Triggers.Ptr(i)->Get_Name()
                );
            }

            return 1;
        })
        .addCFunction("getDeletedTriggerNames", [](auto L) {
            const auto engine = SharedLuaEngine(L);

            auto trigger_names_table = LuaTableBuilder::Push_New_Table(engine);
            auto triggers = TriggerClass::RemovedTriggers;

            for (const auto& trigger_name : triggers) {
                trigger_names_table.With_Index_Value(trigger_name);
            }

            return 1;
        })
        .addCFunction("getTrigger", [](auto L) {
            const auto engine = SharedLuaEngine(L);
            auto arguments = LuaArguments(engine, "<string> Scenario.getTeamType(<string: name>...)");

            arguments.Count_Is(1)
                .First_Argument_Is<std::string>()
                .Assert();

            const auto trigger_name = arguments.Read_First<std::string>().Unpack();

            const auto trigger = TriggerClass::As_Pointer(trigger_name.c_str());

            if (trigger == nullptr) {
                engine.Raise_Error_Format("Trigger not found: {}", trigger_name);
            }

            char trigger_definition[128];

            trigger->Write_INI_String(trigger_definition);

            engine.Push_Value(trigger_definition);

            return 1;
        })
        .addCFunction("addTrigger", [](auto L) {
            const auto engine = SharedLuaEngine(L);
            auto arguments = LuaArguments(engine, "Scenario.addTrigger(<string: name>, <string: definitionCsv>)");

            arguments.Count_Is(2)
                .First_Argument_Is<std::string>()
                .First_Argument_Is<std::string>()
                .Assert();

            const auto name = arguments.Read_First<std::string>().Unpack();
            const auto definition = arguments.Read_Next<std::string>().Unpack();

            arguments.Assert_String_Parameter_Is_Valid("name", name, 4);
            arguments.Assert_String_Parameter_Is_Valid("definition", definition, 127);

            // TODO: Validate method create, call before push
            LuaList.Push<AddTriggerLuaEvent>(name, definition);

            return 0;
        })
        .addCFunction("deleteTriggerIfExists", [](auto L) {
            const auto engine = SharedLuaEngine(L);
            auto arguments = LuaArguments(engine, "<bool> Scenario.deleteTriggerIfExists(<string: name>)");

            arguments.Count_Is(1).First_Argument_Is<std::string>().Assert();

            const auto name = arguments.Read_First<std::string>().Unpack();

            LuaList.Push<DeleteTriggerLuaEvent>(name);

            // return value
            const auto trigger = TriggerClass::As_Pointer(name.c_str());
            const auto trigger_exists = trigger != nullptr;

            engine.Push_Value(trigger_exists);

            return 1;
        });
    });
}
