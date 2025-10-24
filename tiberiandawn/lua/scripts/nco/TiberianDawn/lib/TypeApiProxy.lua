local TypeValidator = require("nco.lib.TypeValidator")

local isNotBlank = TypeValidator.Validators.isNotBlank
local isType = TypeValidator.Validators.isType

---@class RuleSectionApi
---@field __name string
---@field getPropertyNames fun(): string[]
---@field getPropertyType fun(propertyName: string): type
---@field getProperty fun(propertyName: string): number|boolean|string
---@field setProperty fun(propertyName: string, value: number|boolean|string): number|boolean|string

---@alias TypeApiProxy RuleSectionApi | { [string]: number|boolean }

---@param api CppApi
---@param typeName string
---@param getPropertyNamesFunc string
---@param instanceName string
---@return TypeApiProxy
local function TypeApiProxy(api, typeName, getPropertyNamesFunc, instanceName)
  TypeValidator.validateCall("TypeApiProxy", {
    api = {api, isType("table")},
    typeName = {typeName, isType("string"), isNotBlank}
  })

  local getPropertyTypeFunc = string.format("get%PropertyType", typeName)
  local getPropertyValueFunc = string.format("get%PropertyValue", typeName)
  local setPropertyValueFunc = string.format("set%PropertyValue", typeName)

  local function getPropertyType(...)
    return api[getPropertyTypeFunc](instanceName, ...)
  end

  local function getProperty(...)
    return api[getPropertyValueFunc](instanceName, ...)
  end

  local function setProperty(...)
    return api[setPropertyValueFunc](instanceName, ...)
  end

  local function getPropertyNames()
    return api[getPropertyNamesFunc]()
  end

  return setmetatable(
    {
      __name = typeName,
      getPropertyType = getPropertyType,
      getProperty = getProperty,
      setProperty = setProperty,
      getPropertyNames = getPropertyNames,
    },
    {
      -- get value
      __index = function(_, ...)
        return getProperty(...)
      end,
      -- set value
      __newindex = function(_, ...)
        return setProperty(...)
      end
    }
  )
end

---@alias TypesApiProxy CppRulesApi | { [string]: TypeApiProxy }

---@param api CppRulesApi
---@param typeName string
---@return TypesApiProxy
local function TypesApiProxy(api, typeName)
  local getInstanceNamesFunc = string.format("get%InstanceNames", typeName)
  local getPropertyNamesFunc = string.format("get%PropertyNames", typeName)

  return setmetatable(
    {
      getInstanceNames = api[getInstanceNamesFunc],
      getPropertyNames = api[getPropertyNamesFunc]
    },
    {
      __index = function(_, instanceName)
        return TypeApiProxy(api, typeName, getPropertyNamesFunc, instanceName)
      end,
      -- make proxy read only
      __newindex = function()
        error("Type API is read only. Did you mean to set a property and forget to add the type name?")
      end
    }
  )
end

return TypesApiProxy
