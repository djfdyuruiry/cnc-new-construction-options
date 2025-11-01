local TypeValidator = require("nco.lib.TypeValidator")
local Utils = require("nco.lib.Utils")

local isNotBlank = TypeValidator.Validators.isNotBlank
local isType = TypeValidator.Validators.isType

---@class CppTypeApi : CppApi
---@field getTypeNames fun(): string[]

---@class TypeInstanceApi
---@field __name string
---@field getPropertyNames fun(): string[]
---@field getPropertyType fun(): type|string
---@field getProperty fun(propertyName: string): number|boolean|string
---@field setProperty fun(propertyName: string, value: number|boolean|string): number|boolean|string

---@alias TypeInstanceApiProxy TypeInstanceApi | { [string]: number|boolean|string }

---@class TypeApi
---@field __name string
---@field getInstanceNames fun(): string[]
---@field getPropertyNames fun(): string[]

---@alias TypeApiProxy TypeApi | { [string]: TypeInstanceApi }

---@param api CppTypeApi
---@param typeName string
---@param getPropertyNamesFunc string
---@param instanceName string
---@return TypeInstanceApiProxy
local function TypeInstanceApiProxy(api, typeName, getPropertyNamesFunc, instanceName)
  TypeValidator.validateCall("TypeApiProxy", {
    api = {api, isType("table")},
    typeName = {typeName, isType("string"), isNotBlank}
  })

  local getPropertyTypeFunc = string.format("get%sPropertyType", typeName)
  local getPropertyValueFunc = string.format("get%sPropertyValue", typeName)
  local setPropertyValueFunc = string.format("set%sPropertyValue", typeName)

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
      __name = instanceName,
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

---@param api CppTypeApi
---@param typeName string
---@return TypeApiProxy
local function TypeApiProxy(api, typeName)
  local getInstanceNamesFunc = string.format("get%sInstanceNames", typeName)
  local getPropertyNamesFunc = string.format("get%sPropertyNames", typeName)

  if type(api[getInstanceNamesFunc]) ~= "function" then
    local validTypeNames = Utils.arrayToCsv(api.getTypeNames())

    print(getInstanceNamesFunc)
    print(getPropertyNamesFunc)
    error(
      string.format(
        "Invalid type name: %s (Valid type names: %s)",
        typeName,
        validTypeNames
      )
    )
  end

  return setmetatable(
    {
      __name = typeName,
      getInstanceNames = api[getInstanceNamesFunc],
      getPropertyNames = api[getPropertyNamesFunc]
    },
    {
      __index = function(_, instanceName)
        return TypeInstanceApiProxy(api, typeName, getPropertyNamesFunc, instanceName)
      end,
      -- make proxy read only
      __newindex = function()
        error("Type API is read only. Did you mean to set a property and forget to add the type name?")
      end
    }
  )
end

return TypeApiProxy
