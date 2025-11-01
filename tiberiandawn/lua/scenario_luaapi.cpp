#include "../common/lua/luaapi.h"
#include "../common/lua/luaarguments.h"
#include "../common/lua/luatablebuilder.h"

#include "../function.h"

#include "events/addteam_luaevent.h"
#include "events/addtrigger_luaevent.h"
#include "events/deletetrigger_luaevent.h"
#include "events/modifyhousemoney_luaevent.h"
#include "scenario_luaapi.h"

ScenarioLuaApi::ScenarioLuaApi(std::string scenario_name, std::string scenario_type, std::string scenario_faction, std::string scenario_house): TiberianDawnLuaApi("Scenario", true)
{
    ScenarioName = std::move(scenario_name);
    ScenarioType = std::move(scenario_type);
    ScenarioFaction = std::move(scenario_faction);
    ScenarioHouse = std::move(scenario_house);
}

void ScenarioLuaApi::Register_Consts(LuaEngine& engine) const
{
    With_Api_Namespace(engine, [&](auto& n) {
        n.addConstant("name", ScenarioName)
            .addConstant("type", ScenarioType)
            .addConstant("faction", ScenarioFaction)
            .addConstant("house", ScenarioHouse);
    });
}

void ScenarioLuaApi::Register_Functions(LuaEngine& engine) const
{
    With_Api_Namespace(engine, [](auto& n) {
        n.addCFunction("getHouseNames", [](auto L) {
            const auto engine = SharedLuaEngine(L);

            auto house_name_table = LuaTableBuilder(engine);

            for(auto i = HOUSE_FIRST; i < HOUSE_COUNT; i++) {
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

            const auto house = HouseClass::As_Pointer(
                Parse_House_Name(engine, name)
            );

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

            const auto house = Parse_House_Name(engine, name);

            LuaList.Push<ModifyHouseMoneyLuaEvent>(house, money_modifier);

            return 0;
        })
        .addCFunction("getTeamTypeNames", [](auto L) {
            const auto engine = SharedLuaEngine(L);

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
            const auto engine = SharedLuaEngine(L);
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

            // BUG: Game crashes on parse if definition CSV is not valid
            LuaList.Push<AddTeamLuaEvent>(name, definition);

            return 0;
        })
        .addCFunction("getTriggerNames", [](auto L) {
            const auto engine = SharedLuaEngine(L);

            auto trigger_names_table = LuaTableBuilder(engine);
            auto i = 0;

            while (i < Triggers.Count()) {
                const auto trigger_name = Triggers.Ptr(i)->Get_Name();

                trigger_names_table.With_Index_Value(trigger_name);
                i++;
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

            // BUG: Game crashes on parse if definition CSV is not valid
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
            auto trigger_exists = trigger != NULL;

            engine.Push_Value(trigger_exists);

            return 1;
        });
    });
}

HousesType ScenarioLuaApi::Parse_House_Name(const LuaEngine& engine, const std::string& name)
{
    const auto houseType = HouseTypeClass::From_Name(name.c_str());

    if (houseType == HOUSE_NONE) {
        engine.Raise_Error_Format(
            "Failed to parse house name from string: {}",
            name
        );
    }

    return houseType;
}
