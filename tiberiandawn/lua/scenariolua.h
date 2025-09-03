#pragma once

#include <format>
#include <optional>
#include <string>
#include <vector>

#include "../common/lua/logging_luaapi.h"
#include "../common/lua/luaengine.h"
#include "../common/lua/luascripts.h"
#include "../common/lua/rules_luaapi.h"
#include "../common/lua/system_luaapi.h"
#include "../common/logger.h"

#include "../externs.h"
#include "../function.h"
#include "../scenario.h"

#include "game_luaapi.h"
#include "messages_luaapi.h"
#include "scenario_luaapi.h"
#include "ui_luaapi.h"

/**
 * Used to manage Lua runtime from game engine
 * static code.
 */
class ScenarioLua final
{
public:
     static const LuaEngine& Get_Engine() {
          if (!Engine.has_value()) {
               CNC_LOG_FATAL("Attempted to access lua engine before it was loaded");
          }

          return Engine.value();
     }

     /**
      * Scenario has been fully loaded, so time to 
      * initialize Lua runtime with scenario data.
      */
     static void On_Scenario_Load(const CCINIClass& ini, GameEnum game_type, ScenarioClass& scenario, HouseClass* player) {
          Call_Back();

          auto scenario_name = std::string(scenario.ScenarioName);
          std::string scenario_type_name = game_type == GAME_NORMAL ? "single-player" : "multiplayer";
          std::string faction = player->ActLike == HOUSE_GOOD ? "gdi" : "nod";
          auto house_name = std::string(player->Class->IniName);

          std::transform(scenario_name.begin(), scenario_name.end(), scenario_name.begin(), ::tolower);
          std::transform(house_name.begin(), house_name.end(), house_name.begin(), ::tolower);

          CNC_LOG_INFO("Initializing Lua for scenario: {}", scenario_name);

          Engine = LuaEngineBuilder<UniqueLuaEngine>()
               .With_Api<SystemLuaApi>()
               .With_Api<LoggingLuaApi>()
               .With_Api<RulesLuaApi<RuleSectionsProvider>>()
               .With_Api<GameLuaApi>()
               .With_Api<MessagesLuaApi>()
               .With_Api<UiLuaApi>()
               .With_Api<ScenarioLuaApi>(scenario_name, scenario_type_name, faction, house_name)
               .Build();

          Exec_Scenario_Lua_Scripts(ini, scenario, scenario_name, faction, house_name);

          CNC_LOG_DEBUG("Scenario Lua initialization done");

          Call_Back();
     }

     ScenarioLua() = delete;

private:
     static inline std::optional<UniqueLuaEngine> Engine;

     /**
      * Run various lua files for the current scenario, these files are attempted:
      * 
      * - File path in INI '[Basic].LuaScript', defaults to LuaScripts::On_Scenario_Load (can be used as a hook for all scenarios)
      * - Name of the scenario '<scenario_name>.lua', e.x. 'scg01ea.lua' (in lower case)
      * - Name of the scenario without variant, e.x. 'scg01.lua' (again, in lower case)
      * - Name of the player faction '<faction>-scenario.lua', e.x. 'gdi-scenario.lua' (again, in lower case)
      * - Name of the player house '<house>-scenario.lua', e.x. 'goodguy-scenario.lua' (again, in lower case)
      *
      */
     static void Exec_Scenario_Lua_Scripts(
          const CCINIClass& ini,
          ScenarioClass& scenario,
          const std::string& scenario_name,
          const std::string& faction_name,
          const std::string& house_name
     ) {
          auto scenario_lua_script_path = ini.Get_String("Basic", "LuaScript", LuaScripts::On_Scenario_Load);
          auto lua_scripts_to_load = std::vector<std::string> {
               scenario_lua_script_path,
               std::format("{}.lua", scenario_name),
               std::format("{}.lua", scenario_name.substr(0, 5)),
               std::format("{}-scenario.lua", faction_name),
               std::format("{}-scenario.lua", house_name)
          };

          for (const auto& script_path : lua_scripts_to_load) {
               Get_Engine()
                    .Exec_File_If_Exists(script_path)
                    .On_Error([&](auto& r) {
                         CNC_LOG_ERROR(
                              "Failed to load scenario script '{}' due to an error: {}",
                              script_path,
                              r.Error.value_or("unknown error")
                         );
                    });
          }
     }
};
