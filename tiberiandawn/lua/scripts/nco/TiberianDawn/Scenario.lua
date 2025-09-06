local ApiModule = require("nco.lib.ApiModule")

---@class ScenarioPlayer
---@field faction "gdi"|"nod"|"civilian"
---@field house string

---@class ScenarioHouses
---@field getNames fun(): string[]

---@class ScenarioTriggers
---@field getNames fun(): string[]
---@field deleteIfExists fun(triggerName: string): boolean

--[[
  API that provides controls for the current mission
  or skirmish match.

  Provides scenario info, player info and trigger control.
]]
---@class Scenario : ApiModule
---@field name string
---@field type "single-player"|"multiplayer"
---@field player ScenarioPlayer
---@field houses ScenarioHouses
---@field triggers ScenarioTriggers

---@type Game
local Scenario = ApiModule({
  modulePath = { "TiberianDawn", "Scenario" },
  cppApi = "Scenario",
  cppSource = "tiberiandawn/lua/scenario_luaapi.h",
  builder = function(cppApi)
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
          -- TODO: Get house details by name via [] operator
        }
      ),

      triggers = setmetatable(
        {
          getNames = cppApi.getTriggerNames,
          deleteIfExists = cppApi.deleteTriggerIfExists
        },
        {
          -- TODO: Get trigger details by name via [] operator
        }
      )
    }
  end
})

return Scenario
