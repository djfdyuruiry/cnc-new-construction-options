local Tests = {
  ---@type { [string]: { beforeAll: fun(), beforeEach: fun(), tests: { [string]: fun() } } }
  testSuites = {}
}

---@type nil|{ beforeAll: fun(), beforeEach: fun(), tests: { [string]: fun() }, currentPath: string, depth: number }
local currentTestSuite = nil

local function describe(name, func)
  local rootCall = false

  if type(currentTestSuite) == "nil" then
    rootCall = true

    currentTestSuite = {
      beforeAll = function() end,
      beforeEach = function() end,
      tests = {},
      currentPath = "",
      depth = 1
    }
  else
    if currentTestSuite.depth > 1 then
      currentTestSuite.currentPath = currentTestSuite.currentPath .. " > "
    end

    currentTestSuite.currentPath = currentTestSuite.currentPath .. name
    currentTestSuite.depth = currentTestSuite.depth + 1
  end

  func()

  if rootCall then
    Tests.testSuites[name] = {
      beforeAll = currentTestSuite.beforeAll,
      beforeEach = currentTestSuite.beforeEach,
      tests = {}
    }

    for testName, test in pairs(currentTestSuite.tests) do
      Tests.testSuites[name].tests[testName] = test
    end

    currentTestSuite = nil
  end
end

local function beforeAll(func)
  if type(currentTestSuite) == "nil" then
    error("beforeAll() must be called within a describe block")
  elseif currentTestSuite.depth > 1 then
    error("beforeAll() must be called within the root describe block")
  elseif type(func) ~= "function" then
    error("beforeAll() requires a function parameter")
  end

  currentTestSuite.beforeAll = func
end

local function beforeEach(func)
  if type(currentTestSuite) == "nil" then
    error("beforeEach() must be called within a describe block")
  elseif currentTestSuite.depth > 1 then
    error("beforeEach() must be called within the root describe block")
  end

  currentTestSuite.beforeEach = func
end

local function should(name, func)
  if type(currentTestSuite) == "nil" then
    error("should() must be called within a describe block")
  end

  local fullTestName = currentTestSuite.currentPath .. " should " .. name

  currentTestSuite.tests[fullTestName] = func
end

local function runTests()
  local testResults = {}

  for testSuiteName, testSuite in pairs(Tests.testSuites) do
    print("> [" .. testSuiteName .. "]\n")

    local success, err = pcall(testSuite.beforeAll)

    if not success then
      print("❌ TEST SUITE FAILED, beforeAll error: " .. err)

      testResults["[" .. testSuiteName .. "] ALL"] = {
        testSuite = testSuiteName,
        passed = false,
        error = err
      }
    end

    for testName, func in pairs(testSuite.tests) do
      print("\n  > " .. testName .. "\n") -- Print test name

      success, err = pcall(testSuite.beforeEach)

      if not success then
        print("  ❌ TEST '" .. testName .. "' FAILED, beforeEach error: " .. err)

        testResults["[" .. testSuiteName .. "] > " .. testName] = {
          testSuite = testSuiteName,
          passed = false,
          error = "beforeEach: " .. err
        }
      end

      success, err = pcall(func)

      testResults["[" .. testSuiteName .. "] > " ..testName] = {
        testSuite = testSuiteName,
        passed = success and true or false,
        error = success and nil or err
      }

      if success then
        print("\n  ✅ TEST '" .. testName .. "' PASSED")
      else
        print("\n  ❌ TEST '" .. testName .. "' FAILED: " .. tostring(err))
      end
    end
  end

  local passedTests = 0
  local failedTests = 0

  for _, result in pairs(testResults) do
    if result.passed then
      passedTests = passedTests + 1
    else
      failedTests = failedTests + 1
    end
  end

  print("\nTest Summary:\n")
  print("  Passed: " .. passedTests)
  print("  Failed: " .. failedTests)
  print("  Total: " .. (passedTests + failedTests))

  -- Print names of failed tests with test suite prefix
  print("\nFailed Tests:\n")
  for testName, result in pairs(testResults) do
    if not result.passed then
      print(testName)
    end
  end

  os.exit(failedTests)
end

--[[
  Hand rolled micro unit test framework. Use assert
  to check tests, no matchers or other utils are 
  provided.

  beforeAll and beforeEach are only supported in the
  root describe context for now.

  Once all test suites are declared, call runTests to
  execute all suites. This will call os.exit when finished
  with the exit code matching the number of failed tests.

  Example test suite:

  ```lua
    local TestFramework = require("lib.TestFramework")

    local describe = Tests.describe
    local beforeEach = Tests.beforeEach
    local beforeAll = Tests.beforeAll
    local should = Tests.should

    describe("my test suite", function ()
      beforeAll(function ()
        print("before all")
      end)

      beforeEach(function ()
        print("before each")
      end)

      describe("sub thing", function()
        should("my test", function()
          assert(true, "Never gonna happen")
        end)

        should("fail", function ()
          assert(false, "I made it happen")
        end)
      end)
    end)
  ```

]]
---@class TestFramework
---@field describe fun(name: string, func: fun())
---@field beforeAll fun(func: fun())
---@field beforeEach fun(func: fun())
---@field should fun(name: string, func: fun())
---@field runTests fun()

---@type TestFramework
local TestFramework = {
  describe = describe,
  beforeAll = beforeAll,
  beforeEach = beforeEach,
  should = should,
  runTests = runTests
}

return TestFramework
