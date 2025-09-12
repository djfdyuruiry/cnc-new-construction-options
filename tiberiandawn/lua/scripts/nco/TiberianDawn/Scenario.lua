local TdApiModule = require("nco.TiberianDawn.lib.TdApiModule")

---@class House
---@field name string
---@field getMoney fun(): number
---@field giveMoney fun(amount: number)
---@field takeMoney fun(amount: number)

---@class ScenarioPlayer
---@field faction "gdi"|"nod"|"civilian"
---@field house House

---@class ScenarioHouses
---@field getNames fun(): string[]

---@class ScenarioTeamTypes
---@field getNames fun(): string[]

---@class ScenarioTriggers
---@field getNames fun(): string[]
---@field deleteIfExists fun(name: string): boolean

--[[
  API that provides controls for the current mission
  or skirmish match.

  Provides scenario info, player info and trigger control.
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
        getNames = cppApi.getTeamTypeNames
      },
      {
        __index = function (_, teamName)
          return cppApi.getTeamType(teamName)
        end,
        __newindex = function (_, teamName, csvDefinition)
          return cppApi.addTeamType(teamName, csvDefinition)
        end
      }
    ),

    triggers = setmetatable(
      {
        getNames = cppApi.getTriggerNames,
        deleteIfExists = cppApi.deleteTriggerIfExists
      },
      {
        __index = function (_, triggerName)
          return cppApi.getTrigger(triggerName)
        end,
        __newindex = function (_, triggerName, csvDefinition)
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
