local Path = require("nco.lib.Path")

---@alias CallsTable { [string]: { [string]: boolean[]|any[][] } }

---@alias MockCncApi fun(callsHandler: (fun(calls: (fun(): CallsTable)))?, mockHandler: fun(getCalls: (fun(calls: CallsTable)), mock: table))?)

---@class CncApiMock
---@field __calls CallsTable,
---@field __registeredModules ApiModuleSpec[]
---@field __extend MockCncApi
---@field __reset fun()

---@type MockCncApi
---@returns CncApiMock
local mockCncApi

---@param callsHandler fun(calls: CallsTable)?
---@param mockHandler fun(getCalls: (fun(calls: CallsTable)), mock: table)?
mockCncApi = function(callsHandler, mockHandler)
  ---@return CallsTable
  local function buildCallsTable()
    local calls = {
      Logger = {
        getLevel = {},
        setLevel = {},
        log = {}
      },
      Rules = {
        getSectionNames = {},
        getRuleNamesForSection = {},
        getRuleType = {},
        getRuleValue = {},
        setRuleValue = {}
      },
      Event = {},
      System = {
        gamePath = {},
        luaPath = {},
        userPath = {},
        pathSeparator = {},
        isWindows = {}
      }
    }

    if type(callsHandler) == "function" then
      callsHandler(calls)
    end

    return calls
  end

  ---@param getCalls fun(): CallsTable
  local function buildMockTable(getCalls)
    local mock = {
      Logger = setmetatable(
        {},
        {
          __index = function (_, field)
            if field == "getLevel" then
              return function(...)
                table.insert(getCalls().Logger.getLevel, {...})

                return "debug"
              end
            elseif field == "setLevel" then
              return function(...)
                table.insert(getCalls().Logger.setLevel, {...})
              end
            elseif field == "log" then
              return function(...)
                table.insert(getCalls().Logger.log, {...})
                print(...)
              end
            end
          end
        }
      ),
      Rules = setmetatable(
        {},
        {
          -- TODO: hooks to allow test scripts to have a
          --       mock rules table that gets used by below
          __index = function (_, field)
            if field == "getSectionNames" then
              return function(...)
                table.insert(getCalls().Rules.getSectionNames, {...})
                return {"Section1", "Section2"}
              end
            elseif field == "getRuleNamesForSection" then
              return function(...)
                table.insert(getCalls().Rules.getRuleNamesForSection, {...})
                return {"Rule1", "Rule2"}
              end
            elseif field == "getRuleType" then
              return function(...)
                table.insert(getCalls().Rules.getRuleType, {...})
                return "number"
              end
            elseif field == "getRuleValue" then
              return function(...)
                table.insert(getCalls().Rules.getRuleValue, {...})
                return 33
              end
            elseif field == "setRuleValue" then
              return function(...)
                table.insert(getCalls().Rules.setRuleValue, {...})
                return 44
              end
            end
          end
        }
      ),
      Event = setmetatable(
        {},
        {
          __index = function (_, field)
          end
        }
      ),
      System = setmetatable(
        {},
        {
          __index = function (_, field)
            if field == "gamePath" then
              table.insert(getCalls().System.gamePath, true)
              return Path(".", "/", false)
            elseif field == "luaPath" then
              table.insert(getCalls().System.luaPath, true)
              return Path(".", "/", false)
            elseif field == "userPath" then
              table.insert(getCalls().System.userPath, true)
              return Path(".", "/", false)
            elseif field == "pathSeparator" then
              table.insert(getCalls().System.pathSeparator, true)
              return "/"
            elseif field == "isWindows" then
              table.insert(getCalls().System.isWindows, true)
              return false
            elseif field == "Path" then
              return function(...)
                table.insert(getCalls().System.Path, {...})
                return Path(({...})[1], "/", false)
              end
            end
          end
        }
      )
    }

    if type(mockHandler) == "function" then
      mockHandler(getCalls, mock)
    end

    return mock
  end

  local calls = buildCallsTable()

  local function getCalls()
    -- reference upvalue
    return calls
  end

  local mock = buildMockTable(getCalls)
  local registeredModules = {}

  --[[
    Mock implementation of the C++ backend API.

    - To use: Require this module at the start of your test script,
    ApiModule is configured to inject mocks using the global table
    __CNC_API_MOCK and, if present.
    - Note: Always place it as the first require in your script, or mock 
    injection will fail.
    - Field lookups and function calls are recorded, use __calls to inspect
      and assert; params for function calls are captured
    - Modules that get registered via the ApiModule class are recorded in
      __registeredModules.
    - All calls and registered modules can be cleared by calling __reset().
    - Adding additional mocks can be achieved by calling __extend():
      ```lua
        local CncApiMock = require("nco.lib.CncApiMock")

        -- Always call __extend() before requiring modules that depend on the
        -- additional C++ APIs you add to the mock, otherwise these will fail
        -- to load. Any changes here will persist, and can be further extended
        -- by calling __extend() later.
        CncApiMock().__extend(
          function(calls)
            -- amend the calls table
          end,
          function(getCalls, mock)
            -- amend the mock table, use getCalls() to record calls 
          end
        )

        -- reset the mock
        CncApiMock().__reset()
      ```
  ]]
  ---@type CncApiMock
  _G.__CNC_API_MOCK = setmetatable(
    {
      __calls = getCalls,
      ---@type ApiModuleSpec[]
      __registeredModules = registeredModules,
      __extend = mockCncApi,
      __reset = function ()
        calls = buildCallsTable()
      end
    },
    {
      ---@param moduleSpec ApiModuleSpec
      __call = function(_, moduleSpec)
        table.insert(registeredModules, moduleSpec)

        return mock[moduleSpec.name]
      end
    }
  )

  return _G.__CNC_API_MOCK
end

-- ensure Mock is initialized before require returns.
-- nco.lib.ApiModule will detect the global variable if 
-- this file is required before any API modules.
mockCncApi()

--- Lazy loading the mock from a function ensures 
--- any modifications to the global table are not
--- missed; __extend() and __reset() modify it.
---@returns CncApiMock
return function ()
  return _G.__CNC_API_MOCK
end
