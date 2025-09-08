local Path = require("nco.lib.Path")

---@alias CallsTable { [string]: { [string]: boolean[]|any[][] } }

_G.__CNC_API_MOCK = {}

---@type fun(handler: fun(calls: CallsTable, mock: table))
local mockCncApi

mockCncApi = function(handler)
  ---@return CallsTable
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

  ---@param calls CallsTable
  local function buildMockTable(calls)
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
      -- TODO: Rules
      Rules = {},
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

  local calls = buildCallsTable()
  local mock = buildMockTable(calls)
  local registeredModules = {}

  if type(handler) == "function" then
    handler(calls, mock)
  end

  --[[
    Mock implementation of the C++ backend API.

    To use: Require this module at the start of your test script,
    ApiModule is configured to check the global table __CNC_API_MOCK and
    use it if present.

    Records field lookup and function calls (params are captured), use
    __calls to inspect and assert.

    Modules that get registered via the ApiModule class are recorded in
    __registeredModules.

    All calls and registered modules can be cleared by calling __reset().
  
    Adding additional mocks can be achieved by calling the reset method:

      ```lua
        __CNC_API_MOCK.__reset(function(calls, mocks)
          calls.MyModule = {
            -- ...
          }

          mocks.MyModule = {
            -- ...
          }
        end)
      ```
  ]]
  _G.__CNC_API_MOCK = setmetatable(
    {
      __calls = calls,
      ---@type ApiModuleSpec[]
      __registeredModules = registeredModules,
      __reset = mockCncApi
    },
    {
      ---@param moduleSpec ApiModuleSpec
      __call = function(_, moduleSpec)
        table.insert(registeredModules, moduleSpec)

        return mock[moduleSpec.name]
      end
    }
  )
end

mockCncApi()

return _G.__CNC_API_MOCK
