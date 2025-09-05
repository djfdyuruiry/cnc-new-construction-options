local TypeValidator = require("nco.lib.TypeValidator")

local isType = TypeValidator.Validators.isType
local isNotBlank = TypeValidator.Validators.isNotBlank
local isNotEmpty = TypeValidator.Validators.isNotEmpty

local function ApiModule(moduleSpec)
  TypeValidator.validateCall(
    "ApiModule",
    {
      { moduleSpec, isType("table") },
      { moduleSpec.modulePath, isType("table"), isNotEmpty },
      { moduleSpec.cppApi, isType("string"), isNotBlank },
      { moduleSpec.cppSource, isType("string"), isNotBlank },
      { moduleSpec.apiModuleBuilder, isType("function"), isNotBlank }
    }
  )

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
  if type(__CNC_API) ~= "table" or type(__CNC_API[moduleSpec.cppApi]) ~= "table" then
    error(
      string.format(
       "%s API failed to init, required C++ backend not loaded: %s",
       moduleDestStr,
       moduleSpec.cppSource
      )
    )
  end

  -- attempt to build module
  local moduleOrError, status = pcall(moduleSpec.apiModuleBuilder, __CNC_API[moduleSpec.cppApi], moduleSpec)

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
  module.__cpp_source = moduleSpec.cppApi.__cpp_source
  module.__name = moduleSpec.cppApi.__name

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

return ApiModule
