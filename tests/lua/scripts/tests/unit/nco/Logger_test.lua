local TestFramework = require("lib.TestFramework")

local CncApiMock = require("nco.lib.CncApiMock")
local Logger = require("nco.Logger")

local describe = TestFramework.describe
local beforeEach = TestFramework.beforeEach
local beforeAll = TestFramework.beforeAll
local should = TestFramework.should

describe("Logger", function ()
  beforeEach(function ()
    CncApiMock().__reset()
  end)

  describe("log", function()
    should("call C++ API", function()
      Logger.log("debug", "nonsense")

      assert(
        #CncApiMock().__calls().Logger.log == 1,
        "Should have called Logger.log once"
      )
    end)

    should("format log message", function()
      Logger.log("debug", "nonsense %d", 33)

      assert(
        CncApiMock().__calls().Logger.log[1][3] == "nonsense 33",
        "Should have called Logger.log once"
      )
    end)
  end)

  describe("log levels", function()
    should("have a level field", function()
      assert(type(Logger.level) == "string", "Logger.level should be a string")
    end)

    should("have a log function", function()
      assert(type(Logger.log) == "function", "Logger.log should be a function")
    end)

    should("have trace function", function()
      assert(type(Logger.trace) == "function", "Logger.trace should be a function")
    end)

    should("have debug function", function()
      assert(type(Logger.debug) == "function", "Logger.debug should be a function")
    end)

    should("have info function", function()
      assert(type(Logger.info) == "function", "Logger.info should be a function")
    end)

    should("have warning function", function()
      assert(type(Logger.warning) == "function", "Logger.warning should be a function")
    end)

    should("have error function", function()
      assert(type(Logger.error) == "function", "Logger.error should be a function")
    end)

    should("have critical function", function()
      assert(type(Logger.critical) == "function", "Logger.critical should be a function")
    end)
  end)
end)

TestFramework.runTests()