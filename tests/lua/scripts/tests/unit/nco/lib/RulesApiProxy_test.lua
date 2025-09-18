local TestFramework = require("lib.TestFramework")

local _then = TestFramework._then
local beforeEach = TestFramework.beforeEach
local describe = TestFramework.describe
local when = TestFramework.when

local CncApiMock = require("nco.lib.CncApiMock")
local RulesApiProxy = require("nco.lib.RulesApiProxy")

describe("RulesApiProxy", function ()
  ---@type RulesApiProxy
  local testProxy

  beforeEach(function ()
    CncApiMock().__reset()

    local mockRulesApi = CncApiMock()({
      name = "Rules"
    })

    testProxy = RulesApiProxy(mockRulesApi)
  end)

  describe("__index", function()
    when("called with section name", function()
      _then("RulesSectionProxy is returned", function()
        local section = testProxy.SomeSection

        assert(type(section) == "table")
        assert(section.__name == "SomeSection")
      end)
    end)

    when("called with section name amd getRuleNames()", function()
      _then("rules C++ API is called", function()
        testProxy.SomeSection.getRuleNames()

        assert(#CncApiMock().__calls().Rules.getRuleNamesForSection == 1)
      end)
    end)

    when("called with section name amd getRuleType()", function()
      _then("rules C++ API is called", function()
        testProxy.SomeSection.getRuleType("SomeRule")

        assert(#CncApiMock().__calls().Rules.getRuleType == 1)
      end)
    end)

    when("called with section name and rule name", function()
      _then("rule value is returned", function()
        local value = testProxy.SomeSection.TheRule

        assert(value == 33)
      end)

      _then("rules C++ API is called", function()
        local value = testProxy.SomeSection.TheRule

        local calls = CncApiMock().__calls()

        assert(#calls.Rules.getRuleValue == 1)

        local args = calls.Rules.getRuleValue[1]

        assert(args[1] == "SomeSection")
        assert(args[2] == "TheRule")
      end)
    end)

    when("called with section name, rule name and then a value", function()
      _then("rules C++ API is called", function()
        testProxy.SomeSection.TheRule = 11

        local calls = CncApiMock().__calls()

        assert(#calls.Rules.setRuleValue == 1)

        local args = calls.Rules.setRuleValue[1]

        assert(args[1] == "SomeSection")
        assert(args[2] == "TheRule")
        assert(args[3] == 11)
      end)
    end)
  end)

  describe("__newindex", function()
    when("called with value", function()
      _then("throws error", function()
        local status = pcall(function() testProxy.SomeSection = {} end)

        assert(not status)
      end)
    end)
  end)
end)
