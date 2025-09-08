require("nco.lib.CncApiMock")

-- common modules __reset handle
local __reset = _G.__CNC_API_MOCK.__reset

_G.__CNC_API_MOCK.__reset = function()
  -- chain from the original __reset to inherit common mocks
  __reset(function(calls, mock)
    calls.Game = {
      win = {},
      lose = {}
    }

    mock.Game = setmetatable(
      {},
      {
        __index = function (_, k)
          if k == "win" then
            return function(...)
              table.insert(calls.Game.win, {...})
            end
          elseif k == "lose" then
            return function(...)
              table.insert(calls.Game.lose, {...})
            end
          end
        end
      }
    )

    calls.Messages = {
      showToPlayer = {}
    }

    mock.Messages = setmetatable(
      {},
      {
        __index = function (_, k)
          if k == "showToPlayer" then
            return function(...)
              table.insert(calls.Messages.showToPlayer, {...})
            end
          end
        end
      }
    )

    calls.Scenario = {
      name = {},
      type = {},
      faction = {},
      house = {},
      getHouseNames = {},
      getTriggerNames = {},
      deleteTriggerIfExists = {}
    }

    mock.Scenario = setmetatable(
      {},
      {
        __index = function(_, k)
          if k == "name" then
            table.insert(calls.Scenario.name, true)

            return "scg01ea"
          elseif k == "type" then
            table.insert(calls.Scenario.type, true)

            return "single-player"
          elseif k == "faction" then
            table.insert(calls.Scenario.faction, true)

            return "gdi"
          elseif k == "house" then
            table.insert(calls.Scenario.house, true)

            return "goodguy"
          elseif k == "getHouseNames" then
            return function(...)
              table.insert(calls.Scenario.getHouseNames, {...})

              return {"goodguy", "badguy"}
            end
          elseif k == "getTriggerNames" then
            return function(...)
              table.insert(calls.Scenario.getTriggerNames, {...})

              return {"TRI1", "TRI2"}
            end
          elseif k == "deleteTriggerIfExists" then
            return function(...)
              table.insert(calls.Scenario.deleteTriggerIfExists, {...})

              return true
            end
          end
        end
      }
    )

    calls.UI = {
      popupOk = {}
    }

    mock.UI = setmetatable(
      {},
      {
        __index = function (_, k)
          if k == "popupOk" then
            return function(...)
              table.insert(calls.UI.popupOk, {...})
            end
          end
        end
      }
    )
  end)
end

_G.__CNC_API_MOCK.__reset()

--[[
  Extension to the nco.lib.CncApiMock that adds mocks for
  Tiberian Dawn specific modules.
]]
return _G.__CNC_API_MOCK
