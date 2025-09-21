#pragma once

#include <algorithm>
#include <format>
#include <optional>
#include <string>
#include <vector>

#include "../common/lua/event_luaapi.h"
#include "../common/lua/logging_luaapi.h"
#include "../common/lua/luaengine.h"
#include "../common/lua/luascripts.h"
#include "../common/lua/rules_luaapi.h"
#include "../common/lua/system_luaapi.h"
#include "../common/atomicqueue.h"
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
     static const LuaEngine& Get_Engine()
     {
          if (!Engine.has_value()) {
               CNC_LOGGER_FATAL("Attempted to access lua engine before it was loaded");
          }

          return Engine.value();
     }

     /**
      * Scenario has been fully loaded, so time to 
      * initialize Lua runtime with scenario data.
      */
     static void On_Scenario_Load(const CCINIClass& ini, GameEnum game_type, ScenarioClass& scenario, HouseClass* player)
     {
          Call_Back();

          CNC_LOGGER_INFO("Game engine called Lua hook: On_Scenario_Load");

          auto scenario_name = std::string(scenario.ScenarioName);
          std::string scenario_type_name = game_type == GAME_NORMAL ? "single-player" : "multiplayer";
          std::string faction = player->ActLike == HOUSE_GOOD ? "gdi" : "nod";
          auto house_name = std::string(player->Class->IniName);

          std::transform(scenario_name.begin(), scenario_name.end(), scenario_name.begin(), ::tolower);

          CNC_LOG_INFO("Initializing Lua for scenario: {}", scenario_name);

          Init_Tiberian_Dawn_Lua_Engine(
               scenario_name,
               scenario_type_name,
               faction,
               house_name
          );

          // ensure house_name is lowercase for filename use
          std::transform(house_name.begin(), house_name.end(), house_name.begin(), ::tolower);

          Exec_Scenario_Lua_Scripts(ini, scenario, scenario_name, faction, house_name);

          CNC_LOGGER_DEBUG("Scenario Lua initialization done");

          Call_Back();
     }

     #pragma region Triggers

     // TODO: Create similar to call lua events for things other than scenario trigger (on defeated, on building built etc.)
     static bool Exec_Event_Trigger(std::string_view trigger_name, std::string_view event_name)
     {
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

     static bool Exec_Script_Trigger(std::string_view trigger_name, std::string_view script_path)
     {
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

     #pragma endregion

     #pragma region Events

     /**
      * Iterate through FIFO lua events, discarding each after processing.
      */
     static void Process_Lua_Events(AtomicQueue<LuaEvent>& events)
     {
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

     #pragma endregion

private:
     static inline const CncLogger Logger = CncLogger("ScenarioLua");
     static inline std::optional<UniqueLuaEngine> Engine;

     /**
      * API management for TD Lua; think of this like
      * an lite IoC container for the Lua runtime.
      */
     static void Init_Tiberian_Dawn_Lua_Engine(
          std::string& scenario_name,
          std::string& scenario_type_name,
          std::string& faction,
          std::string& house_name
     )
     {
          Engine = LuaEngineBuilder<UniqueLuaEngine>()
               .With_Api<SystemLuaApi>()
               .With_Api<LoggingLuaApi>()
               .With_Api<RulesLuaApi<RuleSectionsProvider>>()
               .With_Api<EventLuaApi>()
               .With_Api<GameLuaApi>()
               .With_Api<MessagesLuaApi>()
               .With_Api<UiLuaApi>()
               .With_Api<ScenarioLuaApi>(scenario_name, scenario_type_name, faction, house_name)
               .Build();
     }

     /**
      * Run various lua script files for the current scenario, scripts are attempted in the following order:
      * 
      *   - File path in @property{LuaScripts::On_Scenario_Load} (can be used as a hook for all scenarios)
      *   - Name of the player faction '<faction>-scenario.lua', e.x. 'gdi-scenario.lua' (again, in lower case)
      *   - Name of the player house '<house>-scenario.lua', e.x. 'goodguy-scenario.lua' (again, in lower case)
      *   - Name of the scenario without variant, e.x. 'scg01.lua' (again, in lower case)
      *   - Name of the scenario '<scenario_name>.lua', e.x. 'scg01ea.lua' (in lower case)
      *   - File path in INI '[Basic].LuaScript' (if present, value in INI is converted to lower case)
      *
      * Note: These are designed to flow from most generic script to the most specific script, so that any
      *       overrides can be applied appropriately and files down the chain are executed.
      */
     static void Exec_Scenario_Lua_Scripts(
          const CCINIClass& ini,
          ScenarioClass& scenario,
          const std::string& scenario_name,
          const std::string& faction_name,
          const std::string& house_name
     )
     {
          auto lua_scripts_to_load = std::vector<std::string> {
               LuaScripts::On_Scenario_Load,
               std::format("{}.lua", scenario_name),
               std::format("{}.lua", scenario_name.substr(0, 5)),
               std::format("{}-scenario.lua", faction_name),
               std::format("{}-scenario.lua", house_name)
          };

          auto ini_script_path = ini.Get_String("Basic", "LuaScript", std::string_view("__NOT_FOUND__"));

          if (ini_script_path != "__NOT_FOUND__") {
               // is not blank
               if (!std::all_of(
                    ini_script_path.begin(), ini_script_path.end(), [](unsigned char c){ return std::isspace(c); }
               )) {
                    CNC_LOGGER_DEBUG("Scenario INI contains [Basic].LuaScript key: {}", ini_script_path);

                    // to lower
                    std::transform(ini_script_path.begin(), ini_script_path.end(), ini_script_path.begin(), ::tolower);

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

     ScenarioLua() = delete;
};
