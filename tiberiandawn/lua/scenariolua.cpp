#include <format>
#include <vector>

#include "../common/lua/event_luaapi.h"
#include "../common/lua/logging_luaapi.h"
#include "../common/lua/luascripts.h"
#include "../common/lua/rules_luaapi.h"
#include "../common/lua/system_luaapi.h"
#include "../common/stringutils.h"

#include "../function.h"

#include "game_luaapi.h"
#include "messages_luaapi.h"
#include "scenario_luaapi.h"
#include "scenariolua.h"
#include "tdtypes_luaapi.h"
#include "ui_luaapi.h"

RuleSections& TdRuleSectionsProvider::Sections() {
    return Rule.Sections;
}

const LuaEngine& ScenarioLua::Get_Engine() {
    if (!Engine.has_value()) {
       CNC_LOGGER_FATAL("Attempted to access lua engine before it was loaded");
    }

    return Engine.value();
}

void ScenarioLua::On_Scenario_Load(const CCINIClass& ini, GameEnum game_type, ScenarioClass& scenario, HouseClass* player) {
    Call_Back();

    CNC_LOGGER_INFO("Game engine called Lua hook: On_Scenario_Load");

    auto scenario_name = std::string(scenario.ScenarioName);
    std::string scenario_type_name = game_type == GAME_NORMAL ? "single-player" : "multiplayer";
    std::string faction = player->ActLike == HOUSE_GOOD ? "gdi" : "nod";
    auto house_name = std::string(player->Class->IniName);

    CncStringUtils::To_Lower(scenario_name);

    CNC_LOG_INFO("Initializing Lua for scenario: {}", scenario_name);

    Init_Tiberian_Dawn_Lua_Engine(
       scenario_name,
       scenario_type_name,
       faction,
       house_name
    );

    // ensure house_name is lowercase for filename use
    CncStringUtils::To_Lower(house_name);

    Exec_Scenario_Lua_Scripts(ini, scenario, scenario_name, faction, house_name);

    CNC_LOGGER_DEBUG("Scenario Lua initialization done");

    Call_Back();
}

bool ScenarioLua::Exec_Event_Trigger(std::string_view trigger_name, std::string_view event_name) {
    auto status = false;

    EventLuaApi::Execute_Event(
       Get_Engine(),
       event_name,
       trigger_name
    )
    .If_Ok([&](auto& r) {
       status = true;
    })
    .On_Error([&](auto& r) {
       CNC_LOGGER_ERROR(
            "Failed to exec lua event '{}' on scenario trigger '{}' due to error: {}",
            event_name,
            trigger_name,
            r.Error_Message()
       );
    });

    return status;
}

bool ScenarioLua::Exec_Script_Trigger(std::string_view trigger_name, std::string_view script_path) {
    auto status = false;

    Get_Engine()
       .Exec_File(script_path)
       .If_Ok([&](auto& r) {
            status = true;
       })
       .On_Error([&](auto& r) {
            CNC_LOGGER_ERROR(
                 "Failed to exec lua script on scenario trigger '{}' due to error: {}",
                 trigger_name,
                 r.Error_Message()
            );
       });

    return status;
}

void ScenarioLua::Process_Lua_Events(AtomicQueue<LuaEvent>& events) {
    events.Access([](auto& q) {
       if (q->size() == 0) {
            CNC_LOGGER_TRACE("No Lua Events to process");
            return;
       }

       CNC_LOGGER_DEBUG("Processing Lua Events");

       while (!q->empty()) {
            q->front()->Execute();
            q->pop();
       }
    });
    }

void ScenarioLua::Init_Tiberian_Dawn_Lua_Engine(std::string& scenario_name, std::string& scenario_type_name, std::string& faction, std::string& house_name) {
    Engine = LuaEngineBuilder<UniqueLuaEngine>()
      .With_Api<SystemLuaApi>()
      .With_Api<LoggingLuaApi>()
      .With_Api<RulesLuaApi<TdRuleSectionsProvider>>()
      .With_Api<EventLuaApi>()
      .With_Api<TiberianDawnTypesLuaApi>()
      .With_Api<GameLuaApi>()
      .With_Api<MessagesLuaApi>()
      .With_Api<UiLuaApi>()
      .With_Api<ScenarioLuaApi>(scenario_name, scenario_type_name, faction, house_name)
      .Build();
    }

void ScenarioLua::Exec_Scenario_Lua_Scripts(const CCINIClass& ini, ScenarioClass& scenario, const std::string& scenario_name, const std::string& faction_name, const std::string& house_name) {
    auto lua_scripts_to_load = std::vector {
      std::string(LuaScripts::On_Scenario_Load),
      std::format("{}.lua", scenario_name),
      std::format("{}.lua", scenario_name.substr(0, 5)),
      std::format("{}-scenario.lua", faction_name),
      std::format("{}-scenario.lua", house_name)
    };
    auto ini_script_path = ini.Get_String("Basic", "LuaScript", std::string_view("__NOT_FOUND__"));

    if (ini_script_path != "__NOT_FOUND__") {
      if (!CncStringUtils::Is_Blank(ini_script_path)) {
          CNC_LOGGER_DEBUG("Scenario INI contains [Basic].LuaScript key: {}", ini_script_path);
          CncStringUtils::To_Lower(ini_script_path);
          lua_scripts_to_load.emplace_back(ini_script_path);
      }
    } else {
      CNC_LOGGER_DEBUG("Scenario INI does not contain a [Basic].LuaScript key");
    }

    for (const auto& script_path : lua_scripts_to_load) {
      Get_Engine()
           .Exec_File_If_Exists(script_path)
           .On_Error([&](auto& r) {
                CNC_LOGGER_ERROR(
                     "Failed to load scenario script '{}' due to an error: {}",
                     script_path,
                     r.Error_Message()
                );
           });
    }
}
