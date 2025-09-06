local TypeValidator = require("nco.lib.TypeValidator")

local skipIfNotPresent = TypeValidator.Validators.skipIfNotPresent
local isType = TypeValidator.Validators.isType
local isNotBlank = TypeValidator.Validators.isNotBlank
local isNotEmpty = TypeValidator.Validators.isNotEmpty

---@class CppApi
---@field __name string
---@field __cppSource string

---@class ApiModuleSpec
---@field modulePath string[]
---@field cppApi string
---@field cppSource string
---@field builder fun(cppApi: CppApi, spec: ApiModuleSpec): table

--- Loads an native Lua API module, using a builder pattern and
--- the option to mock the backend C++ API using CncApiMock.
--- 
--- Returned table is a wrapper around the native Lua module locking
--- it down to be read only.
---@param moduleSpec ApiModuleSpec
return function(moduleSpec)
  TypeValidator.validateCall(
    "ApiModule",
    {
      moduleSpec = { moduleSpec, isType("table") },
      ["moduleSpec.modulePath"] = { moduleSpec.modulePath, isType("table"), isNotEmpty },
      ["moduleSpec.cppApi"] = { moduleSpec.cppApi, isType("string"), isNotBlank },
      ["moduleSpec.cppSource"] = { moduleSpec.cppSource, isType("string"), isNotBlank },
      ["moduleSpec.builder"] = { moduleSpec.builder, isType("function") },
      ["_G.__CNC_API_MOCK"] = { _G.__CNC_API_MOCK, skipIfNotPresent, isType("function") }
    }
  )

  local mockPresent = type(_G.__CNC_API_MOCK) == "table"

  -- build module path, working down to destination table (_G.x[.y]...)
  local moduleDest = _G
  local moduleDestStr = ""

  for i, v in ipairs(moduleSpec.modulePath) do
    moduleDest[v] = moduleDest[v] and moduleDest[v] or {}
    moduleDest = moduleDest[v]

    if i ~= 1 then
      moduleDestStr = moduleDestStr .. "."
    end

    moduleDestStr = moduleDestStr .. v
  end

  -- assert cppApi is loaded into Lua state
  ---@diagnostic disable-next-line: undefined-field
  if not mockPresent and not (type(_G.__CNC_API) == "table" and type(_G.__CNC_API[moduleSpec.cppApi]) == "table") then
    error(
      string.format(
       "%s API failed to init, required C++ backend not loaded: %s",
       moduleDestStr,
       moduleSpec.cppSource
      )
    )
  end

  -- attempt to build module (use a mock cppApi via builder, if present)
  ---@type CppApi
  local cppApi = not mockPresent and _G.__CNC_API[moduleSpec.cppApi] or _G.__CNC_API_MOCK(moduleSpec)[moduleSpec.cppApi]

  if mockPresent and type(cppApi) ~="table" then
    error(
      string.format(
       "%s API failed to init, C++ backend mock builder didn't return a table, actual type returned: %s",
        moduleDestStr,
        type(cppApi)
      )
    )
  end

  local status, moduleOrError = xpcall(function()
    return moduleSpec.builder(cppApi, moduleSpec)
  end, debug.traceback)

  if not status then
    error(
      string.format(
        "Failed to build API module '%s' due to error: %s",
        moduleDestStr,
        moduleOrError
      )
    )
  end

  local module = moduleOrError

  if type(module) ~= "table" then
    error(
      string.format(
        "Builder for API module '%s' did not return a table, actual type returned: %s",
        moduleDestStr,
        type(moduleOrError)
      )
    )
  end

  -- load generic metadata
  module.__cpp_source = cppApi.__cppSource
  module.__name = cppApi.__name

  moduleDest = setmetatable(
    {},
    {
      __index = function (t, k)
        return module[k]
      end,
      __newindex = function ()
        error("API modules are read only. Did you mean to access an API method or field?")
      end
    }
  )

  return moduleDest
end
