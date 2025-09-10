local TdApiModule = require("nco.TiberianDawn.lib.TdApiModule")

---@class ScenarioPlayer
---@field faction "gdi"|"nod"|"civilian"
---@field house string

---@class House
---@field getMoney fun(): number
---@field giveMoney fun(amount: number)
---@field takeMoney fun(amount: number)

---@class ScenarioHouses
---@field getNames fun(): string[]

---@class ScenarioTeamTypes
---@field getNames fun(): string[]
---@field get fun(name: string)
---@field add fun(name: string, definition: string)

---@class ScenarioTriggers
---@field getNames fun(): string[]
---@field get fun(name: string)
---@field add fun(name: string, definition: string)
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
---@field teams ScenarioTeamTypes
---@field triggers ScenarioTriggers

---@return Scenario
local function builder(cppApi)
  return {
    name = cppApi.name,
    type = cppApi.type,

    player = {
      faction = cppApi.faction,
      house = cppApi.house
    },

    houses = setmetatable(
      {
        getNames = cppApi.getHouseNames
      },
      {
        __index = function (_, houseName)
          return {
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
    ),

    teams = setmetatable(
      {
        getNames = cppApi.getTeamTypeNames,
        get = cppApi.getTeamType,
        add = cppApi.addTeamType
      },
      {
        -- TODO: Get trigger details by name via [] operator
      }
    ),

    triggers = setmetatable(
      {
        getNames = cppApi.getTriggerNames,
        get = cppApi.getTrigger,
        add = cppApi.addTrigger,
        deleteIfExists = cppApi.deleteTriggerIfExists
      },
      {
        -- TODO: Get trigger details by name via [] operator
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
