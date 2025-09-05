local TypeValidator = require("nco.lib.TypeValidator")

local isType = TypeValidator.Validators.isType
local isNotBlank = TypeValidator.Validators.isNotBlank
local isNotEmpty = TypeValidator.Validators.isNotEmpty

local function ApiModule(modulePath, cppApi, cppSource, apiModuleBuilder)
  TypeValidator.validateCall(
    "ApiModule",
    {
      { modulePath, isType("table"), isNotEmpty },
      { cppApi, isType("string"), isNotBlank },
      { cppSource, isType("string"), isNotBlank },
      { apiModuleBuilder, isType("function"), isNotBlank }
    }
  )

  local moduleDest = _G
  local moduleDestStr = ""

  for i, v in ipairs(modulePath) do
    -- work down to destination table (_G.[0].[1]...)
    moduleDest[v] = moduleDest[v] and moduleDest[v] or {}
    moduleDest = moduleDest[v]

    if i ~= 1 then
      moduleDestStr = moduleDestStr .. "."
    end

    moduleDestStr = moduleDestStr .. v
  end

  if type(__CNC_API) ~= "table" or type(__CNC_API[cppApi]) ~= "table" then
    error(
      string.format(
       "%s API failed to init, required C++ backend not loaded: %s",
       moduleDestStr,
       cppSource
      )
    )
  end

  local moduleOrError, status = pcall(apiModuleBuilder, __CNC_API[cppApi])

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

  module.__cpp_source = cppApi.__cpp_source
  module.__name = cppApi.__name

  moduleDest = setmetatable(
    module,
    {
      __newindex = function ()
        error("API modules are read only. Did you mean to access an API method or field?")
      end
    }
  )

  return moduleDest
end

return ApiModule
