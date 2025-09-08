require("nco.lib.CncApiMock")

local function buildCallsTable()

end

local __reset = _G.__CNC_API_MOCK.__reset

_G.__CNC_API_MOCK.__reset = function()
  ---@type CallsTable
  local calls
  ---@type table
  local mock

  __reset(function(commonCalls, commonMock)
    calls = commonCalls
    mock = commonMock
  end)

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

  -- TODO: Scenario

  calls.UI = {
    showPopup = {},
    showOkPopup = {}
  }

  mock.UI = setmetatable(
    {},
    {
      __index = function (_, k)
        if k == "showPopup" then
          return function(...)
            table.insert(calls.UI.showPopup, {...})
          end
        elseif k == "showOkPopup" then
          return function(...)
            table.insert(calls.UI.showOkPopup, {...})
          end
        end
      end
    }
  )
end

return _G.__CNC_API_MOCK
