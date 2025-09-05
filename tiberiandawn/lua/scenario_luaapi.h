#pragma once

#include <string>

#include "../common/lua/luaapi.h"
#include "../common/lua/luaarguments.h"
#include "../common/lua/luaengine.h"
#include "../common/lua/luatablebuilder.h"

#include "../externs.h"

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
            n.addCFunction("getTriggerNames", [](auto L) {
                 auto engine = SharedLuaEngine(L);

                 auto trigger_names_table = LuaTableBuilder(engine);
                 auto i = 0;

                 while (i < Triggers.Count()) {
                    auto trigger_name = std::string_view(Triggers.Ptr(i)->Get_Name());

                    trigger_names_table.With_Index_Value(trigger_name);
                    i++;
                 }

                 return 1;
             }).addCFunction("deleteTriggerIfExists", [](auto L) {
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
    virtual const char* Get_Cpp_Source() const override {
        return __FILE__;
    }

private:
    std::string Scenario_Name;
    std::string Scenario_Type;
    std::string Scenario_Faction;
    std::string Scenario_House;
};
