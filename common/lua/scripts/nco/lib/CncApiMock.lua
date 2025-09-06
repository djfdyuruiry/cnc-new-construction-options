local Path = require("nco.lib.Path")

--[[
  Mock implementation of the C++ backend API.

  To use: Simply require this module at the start of your test script,
  ApiModule is configured to look the global table __CNC_API_MOCK and
  use it if present.

  Records field lookup and function calls (params are captured), use
  __calls to inspect and assert.

  Modules that get registered via the ApiModule class are recorded in
  __registeredModules.

  All calls and registered modules can be cleared by calling __reset().
]]
_G.__CNC_API_MOCK = (function()
---@return { [string]: { [string]: boolean[]|any[][] } }
  local function buildCallsTable()
    return {
      Logger = {
        level = {},
        log = {}
      },
      System = {
        gamePath = {},
        luaPath = {},
        userPath = {},
        pathSeparator = {},
        isWindows = {}
      }
    }
  end

  local calls = buildCallsTable()
  local registeredModules = {}

  return setmetatable(
    {
      __calls = calls,
      ---@type ApiModuleSpec[]
      __registeredModules = registeredModules,
      __reset = function()
        calls = buildCallsTable()
        registeredModules = {}

        _G.__CNC_API_MOCK.__calls = calls
        _G.__CNC_API_MOCK.__registeredModules = registeredModules
      end
    },
    {
      ---@param moduleSpec ApiModuleSpec
      __call = function(moduleSpec)
        table.insert(registeredModules, moduleSpec)

        return {
          Logger = setmetatable(
            {},
            {
              __index = function (_, field)
                if field == "level" then
                  table.insert(calls.Logger.level, true)
                  return "debug"
                elseif field == "log" then
                  return function(...)
                    table.insert(calls.Logger.log, {...})
                    print(...)
                  end
                end
              end
            }
          ),
          System = setmetatable(
            {},
            {
              __index = function (_, field)
                if field == "gamePath" then
                  table.insert(calls.System.gamePath, true)
                  return Path("/game", "/", false)
                elseif field == "luaPath" then
                  table.insert(calls.System.luaPath, true)
                  return Path("/game/lua", "/", false)
                elseif field == "userPath" then
                  table.insert(calls.System.userPath, true)
                  return Path("/user", "/", false)
                elseif field == "pathSeparator" then
                  table.insert(calls.System.pathSeparator, true)
                  return "/"
                elseif field == "isWindows" then
                  table.insert(calls.System.isWindows, true)
                  return false
                elseif field == "Path" then
                  return function(...)
                    table.insert(calls.System.Path, {...})
                    return Path(({...})[1], "/", false)
                  end
                end
              end
            }
          )
        }
      end
    }
  )
end)()

return _G.__CNC_API_MOCK
