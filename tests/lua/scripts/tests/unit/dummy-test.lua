local TestFramework = require("lib.TestFramework")

local describe = TestFramework.describe
local beforeEach = TestFramework.beforeEach
local beforeAll = TestFramework.beforeAll
local should = TestFramework.should

describe("my fake test", function ()
  beforeAll(function ()
    print("before all")
  end)

  beforeEach(function ()
    print("before each")
  end)

  describe("sub thing", function()
    should("do something", function()
      assert(true, "Never gonna happen")
    end)

    should("fail", function ()
      assert(false, "I made it happen")
    end)
  end)
end)

TestFramework.runTests()
