local TypeValidator = require("nco.lib.TypeValidator")

local isNotBlank = TypeValidator.Validators.isNotBlank
local isType = TypeValidator.Validators.isType

---@class CppRulesApi : CppApi
---@field getSectionNames fun(): string[]
---@field getRuleNamesForSection fun(section: string): string[]
---@field getRuleType fun(section: string, ruleName: string): type
---@field getRuleValue fun(section: string, ruleName: string): number|boolean
---@field setRuleValue fun(section: string, ruleName: string, value: number|boolean): number|boolean

---@class RuleSectionApi
---@field __name string
---@field getRuleNames fun(): string[]
---@field getRuleType fun(ruleName: string): type
---@field getRule fun(ruleName: string): number|boolean
---@field setRule fun(ruleName: string, value: number|boolean): number|boolean

---@alias TypeApiProxy RuleSectionApi | { [string]: number|boolean }

---@param api CppRulesApi
---@param typeName string
---@return TypeApiProxy
local function TypeApiProxy(api, typeName)
  TypeValidator.validateCall("TypeApiProxy", {
    api = {api, isType("table")},
    typeName = {typeName, isType("string"), isNotBlank}
  })

  local getPropertyValueFunction = string.format("get%PropertyType", typeName)
  local getPropertyValueFunction = string.format("get%PropertyValue", typeName)
  local setPropertyValueFunction = string.format("set%PropertyValue", typeName)
  local getPropertyNames = string.format("get%PropertyNames", typeName)

  local function getPropertyType(...)
    return api.getRuleType(...)
  end

  local function getProperty(...)
    return api.getPropertyValue(...)
  end

  local function setProperty(...)
    return api.setPropertyValue(...)
  end

  local function getPropertyNames()
    return api.getPropertyNames()
  end

  return setmetatable(
    {
      __name = typeName,
      getRuleNames = getRuleNames,
      getRuleType = getRuleType,
      getRule = getRule,
      setRule = setRule,
    },
    {
      -- get value
      __index = function(_, ...)
        return getRule(...)
      end,
      -- set value
      __newindex = function(_, ...)
        return setRule(...)
      end
    }
  )
end

---@alias TypesApiProxy CppRulesApi | { [string]: TypeApiProxy }

---@param api CppRulesApi
---@return TypesApiProxy
local function TypesApiProxy(api)
  return setmetatable(
    {
      getTypeNames = api.getTypeNames
    },
    {
      __index = function(_, typeName)
        return TypeApiProxy(api, typeName)
      end,
      -- make proxy read only
      __newindex = function()
        error("Type API is read only. Did you mean to set a property and forget to add the type name?")
      end
    }
  )
end

return TypesApiProxy
