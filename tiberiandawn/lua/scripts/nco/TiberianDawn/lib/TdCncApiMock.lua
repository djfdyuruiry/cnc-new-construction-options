require("nco.lib.CncApiMock")

local function extendCallsTable(calls)
  calls.Game = {
    win = {},
    lose = {}
  }

  calls.Messages = {
    showToPlayer = {}
  }

  calls.Scenario = {
    name = {},
    type = {},
    faction = {},
    house = {},
    getHouseNames = {},
    getTriggerNames = {},
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
        if k == "showToPlayer" then
          return function(...)
            table.insert(getCalls().Messages.showToPlayer, {...})
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
        elseif k == "getTriggerNames" then
          return function(...)
            table.insert(getCalls().Scenario.getTriggerNames, {...})

            return {"TRI1", "TRI2"}
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

_G.__CNC_API_MOCK.__extend(extendCallsTable, extendMockTable)

--[[
  Extension to the nco.lib.CncApiMock that adds mocks for
  Tiberian Dawn specific modules.
]]
return _G.__CNC_API_MOCK
