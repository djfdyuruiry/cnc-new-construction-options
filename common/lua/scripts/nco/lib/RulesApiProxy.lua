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

  local function getRuleNames(...)
    return __CNC_API.Rules.getRuleNamesForSection(sectionName, ...)
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

local function RulesApiProxy(api)
  return setmetatable(
    api,
    {
      __index = function(_, sectionName)
        return RulesSectionProxy(api, sectionName)
      end,
      -- make proxy read only
      __newindex = function()
        error("Rule API is read only. Did you mean to set a rule and forgot to add the name?")
      end
    }
  )
end

return RulesApiProxy
