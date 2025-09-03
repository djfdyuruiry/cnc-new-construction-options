#pragma once

#include <string>

#include "../common/lua/luaapi.h"
#include "../common/lua/luaarguments.h"
#include "../common/lua/luaengine.h"
#include "../common/lua/luatablebuilder.h"

#include "../externs.h"

class ScenarioLuaApi : public LuaApi
{
public:
    ScenarioLuaApi(const LuaEngine& engine, std::string name, std::string type, std::string faction, std::string house)
        : LuaApi(engine, "Scenario", {"Scenario.lua"})
    {
        Name = name;
        Type = type, Faction = faction;
        House = house;
    }

    virtual void Register_Consts() const override
    {
        With_Api_Namespace([&](auto& n) {
            n.addConstant("name", Name)
                .addConstant("type", Type)
                .addConstant("faction", Faction)
                .addConstant("house", House);
        });
    }

    virtual void Register_Functions() const override
    {
        With_Api_Namespace([](auto& n) {
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

private:
    std::string Name;
    std::string Type;
    std::string Faction;
    std::string House;
};
