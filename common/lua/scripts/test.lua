_G.__CNC_API_MOCK = (function(moduleSpec)
  local calls = {
    Logger = {
      level = {},
      logger = {}
    },
    System = {
      gamePath = {},
      luaPath = {},
      userPath = {},
      pathSeparator = {},
      isWindows = {}
    }
  }

  local moduleSpec = {}

  return setmetatable(
    {
      __calls = calls,
      __module_spec = moduleSpec
    },
    {
      __call = function(passedModuleSpec)
        for k,v in pairs(passedModuleSpec) do
          moduleSpec[k] = v
        end
 
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
                    table.insert(calls.Logger.logger, {...})
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

local Logger = require("nco.Logger")

Logger.debug("Get better %s", "mate")
