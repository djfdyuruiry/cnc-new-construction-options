local Utils = require("nco.lib.Utils")
local Logger = require("nco.Logger")

local TdApiModule = require("nco.TiberianDawn.lib.TdApiModule")

--[[
  House management for current Scenario.

  - Manage money: get current money, give money, take money away

  Note: Money in this context is a sum of the credits and value of tiberium in silos
]]
---@class House
---@field name string
---@field getMoney fun(): number
---@field giveMoney fun(amount: number)
---@field takeMoney fun(amount: number)

--[[
  Info on the player for the current scenario.

  See: nco.TiberianDawn.House
]]
---@class ScenarioPlayer
---@field faction "gdi"|"nod"|"civilian"
---@field house House

---@class ScenarioHouses
---@field getNames fun(): string[]

---@class ScenarioTeamTypes
---@field getNames fun(): string[]
---@field exists fun(name: string): boolean

--[[
  Manage triggers for the current scenario.
]]
---@class ScenarioTriggers
---@field getNames fun(): string[]
---@field getDeletedNames fun(): string[]
---@field exists fun(name: string): boolean
---@field deleteIfExists fun(name: string): boolean

--[[
  API that provides controls for the current mission
  or skirmish match.

  - Manage Team Types with `teams`
    ```lua
      -- lookup existing team
      local TM1 = Scenario.teams.TM1

      -- add team
      Scenario.teams.LUA1 = "GoodGuy,0,0,0,0,0,7,3,0,0,2,HTNK:1,LST:1,0,1,1"
    ```
  - Manage Triggers with `triggers`
    ```lua
      -- lookup existing trigger
      local TMR1 = Scenario.triggers.TMR1

      -- add a trigger
      Scenario.triggers.TMR4 = "Time,Reinforce.,8,GoodGuy,LUA1,0"

      -- delete a trigger
      Scenario.triggers.deleteTriggerIfExists("TMR4")
    ```
  - Manage Houses with `houses` (and `player`)
    ```lua
      -- give badguy 2000 credits
      Scenario.houses.goodguy.giveMoney(2000)

      -- take 500 credits away from the player
      Scenario.player.house.takeMoney(500)
    ```

  **Note on Duplicate Teams & Triggers**

  Triggers and teams are identified by a name (e.x. LUA1,TMR1) - names are unique, so a team/trigger with the same name
  can only be added once. The Lua API will prevent any attempts to add duplicate teams/triggers.

  - You can explicitly check if a team exists by calling: `local exists = Scenario.teams.exists("NAME")`
  - You can explicitly check if a trigger exists by calling: `local exists = Scenario.triggers.exists("NAME")`

  Triggers can also be deleted during gameplay (via other Triggers or Lua Scripts), this is used to provide gameplay
  behaviour like stopping Airstrikes if you destroy the Comm Center.

  **Even if a trigger is deleted, the Lua API will prevent any attempt to recreate the trigger. This ensures that Lua
  scripts executed after loading a save game don't re-add already deleted triggers.**
]]
---@class Scenario : ApiModule
---@field name string
---@field type "single-player"|"multiplayer"
---@field player ScenarioPlayer
---@field houses ScenarioHouses | { [string]: House }
---@field teams ScenarioTeamTypes | { [string]: string }
---@field triggers ScenarioTriggers | { [string]: string }

---@return Scenario
local function builder(cppApi)
  local houses = setmetatable(
    {
      getNames = cppApi.getHouseNames
    },
    {
      __index = function (_, houseName)
        return {
          name = houseName,
          getMoney = function ()
            return cppApi.getHouseMoney(houseName)
          end,
          giveMoney = function(amount)
            cppApi.modifyHouseMoney(houseName, amount)
          end,
          takeMoney = function(amount)
            cppApi.modifyHouseMoney(houseName, -amount)
          end
        }
      end
    }
  )

  -- Does a team currently exist with the same name?
  local function teamExists(teamName)
    return Utils.arrayContains(cppApi.getTeamTypeNames(), teamName)
  end

  -- Has a trigger ever existed in the current scenario with the same name?
  local function triggerExists(triggerName)
    -- search deleted triggers first (speeds up search since this is more likely)
    return Utils.arrayContains(cppApi.getDeletedTriggerNames(), triggerName) or
      Utils.arrayContains(cppApi.getTriggerNames(), triggerName)
  end

  return {
    name = cppApi.name,
    type = cppApi.type,

    player = {
      faction = cppApi.faction,
      house = houses[cppApi.house]
    },

    houses = houses,

    teams = setmetatable(
      {
        getNames = cppApi.getTeamTypeNames,
        exists = teamExists
      },
      {
        __index = function (_, teamName)
          return cppApi.getTeamType(teamName)
        end,
        __newindex = function (_, teamName, csvDefinition)
          if (teamExists(teamName)) then
              Logger.warning("Ignoring add team request, team with same name already exists: %s", teamName)
              return
          end

          return cppApi.addTeamType(teamName, csvDefinition)
        end
      }
    ),

    triggers = setmetatable(
      {
        getNames = cppApi.getTriggerNames,
        getDeletedNames = cppApi.getDeletedTriggerNames,
        exists = triggerExists,
        deleteIfExists = cppApi.deleteTriggerIfExists
      },
      {
        __index = function (_, triggerName)
          return cppApi.getTrigger(triggerName)
        end,
        __newindex = function(_, triggerName, csvDefinition)
          if (triggerExists(triggerName)) then
            Logger.warning(
              "Ignoring add trigger request, trigger name was previously used in current scenario: %s",
              triggerName
            )
            return
          end

          return cppApi.addTrigger(triggerName, csvDefinition)
        end
      }
    )
  }
end

---@type Scenario
_G.Scenario = TdApiModule({
  name = "Scenario",
  cppSource = "tiberiandawn/lua/scenario_luaapi.h",
  builder = builder
})

return _G.Scenario
