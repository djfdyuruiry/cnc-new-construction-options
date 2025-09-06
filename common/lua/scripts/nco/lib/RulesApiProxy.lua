---@class CppRulesApi
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

---@alias RulesSectionProxy RuleSectionApi | { [string]: number|boolean }

---@param api CppRulesApi
---@param sectionName string
---@return RulesSectionProxy
local function RulesSectionProxy(api, sectionName)
  local function getRuleType(...)
    return api.getRuleType(sectionName, ...)
  end

  local function getRule(...)
    return api.getRuleValue(sectionName, ...)
  end

  local function setRule(...)
    return api.setRuleValue(sectionName, ...)
  end

  local function getRuleNames()
    return api.getRuleNamesForSection(sectionName)
  end

  return setmetatable(
    {
      __name = sectionName,
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

---@alias RulesApiProxy CppRulesApi | { [string]: RulesSectionProxy }

---@param api CppRulesApi
---@return RulesApiProxy
local function RulesApiProxy(api)
  return setmetatable(
    api,
    {
      __index = function(_, sectionName)
        return RulesSectionProxy(api, sectionName)
      end,
      -- make proxy read only
      __newindex = function()
        error("Rule API is read only. Did you mean to set a rule and forgot to add the section?")
      end
    }
  )
end

return RulesApiProxy
