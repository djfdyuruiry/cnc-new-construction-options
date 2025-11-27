#pragma once

#include <optional>
#include <string>

#include "../common/lua/luaengine.h"
#include "../common/atomicqueue.h"
#include "../common/logger.h"

#include "../scenario.h"

/**
 * Adapter for Lua API to pull in static RulesClass variable.
 *
 * See: concept `RuleSectionsProviderConcept` in @file{common/lua/rules_luaapi.h}
 */
class TdRuleSectionsProvider final
{
public:
    static RuleSections& Sections();

private:
    TdRuleSectionsProvider() = delete;
};

/**
 * Used to manage Lua runtime from game engine static code.
 *
 * This class is called from:
 *
 *   - scenarioini.cpp => when loading a scenario from an INI file
 *   - saveload.cpp    => when loading a scenario from a save file
 *   - scenario.cpp    => when clearing the scenario state
 *
 */
class ScenarioLua final
{
public:
    static const LuaEngine& Get_Engine();

    /**
     * Scenario has been fully loaded, so time to
     * initialize Lua runtime with scenario data.
     */
    static void On_Scenario_Load(
        const GameEnum& game_type,
        const ScenarioClass& scenario,
        const HouseClass& player,
        const std::optional<std::string>& ini
    );

    static void On_Scenario_Load(
        const GameEnum& game_type,
        const ScenarioClass& scenario,
        const HouseClass& player,
        const CCINIClass& ini
    );

    static void On_Scenario_Load(
        const GameEnum& game_type,
        const ScenarioClass& scenario,
        const HouseClass& player
    );

    /**
     * Scenario state is being reset, so we need to ensure
     * any existing Lua runtime state is destroyed.
     */
    static void On_Clear_Scenario();

#pragma region Triggers

    // TODO: Create similar to call lua events for things other than scenario trigger (on defeated, on building built etc.)
    static bool Exec_Event_Trigger(std::string_view trigger_name, std::string_view event_name);

    static bool Exec_Script_Trigger(std::string_view trigger_name, std::string_view script_path);

#pragma endregion

#pragma region Events

    /**
     * Iterate through FIFO lua events, discarding each after processing.
     */
    static void Process_Lua_Events(AtomicQueue<LuaEvent>& events);

#pragma endregion

private:
    static inline const auto& Logger = CncLogger::For(ScenarioLua);
    static inline std::optional<UniqueLuaEngine> Engine;

    /**
     * API management for TD Lua; think of this like
     * a lite IoC container for the Lua runtime.
     */
    static void Init_Tiberian_Dawn_Lua_Engine(
         std::string& scenario_name,
         std::string& scenario_type_name,
         std::string& faction,
         std::string& house_name
    );

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
        const std::optional<std::string>& ini_script_path,
        const ScenarioClass& scenario,
        const std::string& scenario_name,
        const std::string& faction_name,
        const std::string& house_name
    );

     ScenarioLua() = delete;
};
