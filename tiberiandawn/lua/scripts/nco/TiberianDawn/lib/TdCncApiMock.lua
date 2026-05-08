local CncApiMock = require("nco.lib.CncApiMock")

local function extendCallsTable(calls)
  calls.Game = {
    win = {},
    lose = {}
  }

  calls.Messages = {
    setColour = {},
    resetColour = {},
    setMessageTimeout = {},
    resetMessageTimeout = {},
    sendToPlayer = {}
  }

  calls.Scenario = {
    name = {},
    type = {},
    faction = {},
    house = {},
    getHouseNames = {},
    getHouseMoney = {},
    modifyHouseMoney = {},
    getTeamTypeNames = {},
    getTeamType = {},
    addTeamType = {},
    getTriggerNames = {},
    getDeletedTriggerNames = {},
    getTrigger = {},
    addTrigger = {},
    deleteTriggerIfExists = {}
  }

  calls.UI = {
    popupOk = {}
  }
end

local function extendMockTable(getCalls, mock)
  mock.Game = setmetatable(
    {},
    {
      __index = function (_, k)
        if k == "win" then
          return function(...)
            table.insert(getCalls().Game.win, {...})
          end
        elseif k == "lose" then
          return function(...)
            table.insert(getCalls().Game.lose, {...})
          end
        end
      end
    }
  )

  mock.Messages = setmetatable(
    {},
    {
      __index = function (_, k)
        if k == "setColour" then
          return function(...)
            table.insert(getCalls().Messages.setColour, {...})
          end
        elseif k == "resetColour" then
          return function(...)
            table.insert(getCalls().Messages.resetColour, {...})
          end
        elseif k == "setMessageTimeout" then
          return function(...)
            table.insert(getCalls().Messages.setMessageTimeout, {...})
          end
        elseif k == "resetMessageTimeout" then
          return function(...)
            table.insert(getCalls().Messages.resetMessageTimeout, {...})
          end
        elseif k == "sendToPlayer" then
          return function(...)
            table.insert(getCalls().Messages.sendToPlayer, {...})
          end
        end
      end
    }
  )

  mock.Scenario = setmetatable(
    {},
    {
      __index = function(_, k)
        if k == "name" then
          table.insert(getCalls().Scenario.name, true)

          return "scg01ea"
        elseif k == "type" then
          table.insert(getCalls().Scenario.type, true)

          return "single-player"
        elseif k == "faction" then
          table.insert(getCalls().Scenario.faction, true)

          return "gdi"
        elseif k == "house" then
          table.insert(getCalls().Scenario.house, true)

          return "GoodGuy"
        elseif k == "getHouseNames" then
          return function(...)
            table.insert(getCalls().Scenario.getHouseNames, {...})

            return {"GoodGuy", "BadGuy"}
          end
        elseif k == "getHouseNames" then
          return function(...)
            table.insert(getCalls().Scenario.getHouseNames, {...})

            return {"GoodGuy", "BadGuy"}
          end
        elseif k == "getHouseMoney" then
          return function(...)
            table.insert(getCalls().Scenario.getHouseMoney, {...})

            return 1000
          end
        elseif k == "modifyHouseMoney" then
          return function(...)
            table.insert(getCalls().Scenario.modifyHouseMoney, {...})
          end
        elseif k == "getTeamTypeNames" then
          return function(...)
            table.insert(getCalls().Scenario.getTeamTypeNames, {...})

            return { "GDIR1", "GDIR2" }
          end
        elseif k == "getTeamType" then
          return function(...)
            table.insert(getCalls().Scenario.getTeamType, {...})

            return "GoodGuy,0,0,0,0,0,7,3,0,0,2,JEEP:1,LST:1,0,1,1"
          end
        elseif k == "addTeamType" then
          return function(...)
            table.insert(getCalls().Scenario.addTeamType, {...})
          end
        elseif k == "getTriggerNames" then
          return function(...)
            table.insert(getCalls().Scenario.getTriggerNames, {...})

            return { "RNF1", "RNF2" }
          end
        elseif k == "getDeletedTriggerNames" then
          return function(...)
            table.insert(getCalls().Scenario.getDeletedTriggerNames, {...})

            return { "OLD1", "OLD2" }
          end
        elseif k == "getTrigger" then
          return function(...)
            table.insert(getCalls().Scenario.getTriggerNames, {...})

            return "Time,Reinforce.,3,GoodGuy,GDIR1,0"
          end
        elseif k == "addTrigger" then
          return function(...)
            table.insert(getCalls().Scenario.addTrigger, {...})
          end
        elseif k == "deleteTriggerIfExists" then
          return function(...)
            table.insert(getCalls().Scenario.deleteTriggerIfExists, {...})

            return true
          end
        end
      end
    }
  )

  mock.Types = setmetatable(
    {},
    {
      __index = function(_, k)
        if k == "getTypeNames" then
          return function(...)
            return { "Type1", "Type2" }
          end
        elseif k:match([[get.+InstanceNames]]) then
          return function(...)
            return { "Instance1", "Instance2" }
          end
        elseif k:match([[get.+PropertyNames]]) then
          return function(...)
            return { "Property1", "Property2" }
          end
        elseif k:match([[get.+PropertyType]]) then
          return function(...)
            return "number"
          end
        elseif k:match([[get.+PropertyValue]]) then
          return function(...)
            return 33
          end
        elseif k:match([[set.+PropertyValue]]) then
          return function(...)
            return 44
          end
        end
      end
    }
  )

  mock.UI = setmetatable(
    {},
    {
      __index = function (_, k)
        if k == "popupOk" then
          return function(...)
            table.insert(getCalls().UI.popupOk, {...})
          end
        end
      end
    }
  )
end

CncApiMock().__extend(extendCallsTable, extendMockTable)

--[[
  Extension to the nco.lib.CncApiMock that adds mocks for
  Tiberian Dawn specific modules.
]]
return CncApiMock
