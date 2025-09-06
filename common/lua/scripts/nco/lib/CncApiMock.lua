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

_G.__CNC_API_MOCK = (function()
  local calls = buildCallsTable()
  local registeredModules = {}

  return setmetatable(
    {
      __calls = calls,
      __registeredModules = registeredModules,
      __reset = function()
        calls = buildCallsTable()
        registeredModules = {}

        _G.__CNC_API_MOCK.__calls = calls
        _G.__CNC_API_MOCK.__registeredModules = registeredModules
      end
    },
    {
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
                  return "/game"
                elseif field == "luaPath" then
                  table.insert(calls.System.luaPath, true)
                  return "/game/lua"
                elseif field == "userPath" then
                  table.insert(calls.System.userPath, true)
                  return "/user"
                elseif field == "pathSeparator" then
                  table.insert(calls.System.pathSeparator, true)
                  return "/"
                elseif field == "isWindows" then
                  table.insert(calls.System.isWindows, true)
                  return false
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
