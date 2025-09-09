local Tests = {
  ---@type { [string]: { beforeAll: fun(), beforeEach: fun(), tests: { [string]: fun() } } }
  testSuites = {}
}

---@type nil|{ beforeAll: fun(), beforeEach: fun(), tests: { [string]: fun() }, currentPathParts: string[], depth: number }
local currentTestSuite = nil

local function describe(name, func)
  local rootCall = false

  if type(currentTestSuite) == "nil" then
    rootCall = true

    -- describe/doTest blocks tracker for 'recursion'
    currentTestSuite = {
      beforeAll = function() end,
      beforeEach = function() end,
      tests = {},
      currentPathParts = {},
      depth = 0
    }
  else
    -- entered a describe block, push it's name
    currentTestSuite.depth = currentTestSuite.depth + 1
    currentTestSuite.currentPathParts[currentTestSuite.depth] = name
  end

  -- call nested describe block or doTest block
  func()

  if not rootCall and currentTestSuite.depth > 0 then
    -- left a describe block, pop it's name
    table.remove(currentTestSuite.currentPathParts, currentTestSuite.depth)
    currentTestSuite.depth = currentTestSuite.depth - 1
  end

  if rootCall then
    -- end of describe root call, load tests into suite
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

local function when(name, func)
  return describe(string.format("{when} %s", name), func)
end

local function _and(name, func)
  return describe(string.format("{and} %s", name), func)
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

local function doTest(name, func)
  if type(currentTestSuite) == "nil" then
    error("doTest() must be called within a describe block")
  end

  local fullTestName = ""

  -- build full name from nested block names
  for i = 1, currentTestSuite.depth do
    if i > 1 then
      fullTestName = fullTestName .. " > "
    end

    fullTestName = fullTestName .. currentTestSuite.currentPathParts[i]
  end

  fullTestName = fullTestName .. " > " .. name

  currentTestSuite.tests[fullTestName] = func
end

local function should(name, func)
  return doTest(string.format("{should} %s", name), func)
end

local function _then(name, func)
  return doTest(string.format("{then} %s", name), func)
end

local function is(name, func)
  return doTest(string.format("{is} %s", name), func)
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
        print("\n  ✅ TEST PASSED")
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
  print("  Total: " .. tostring(passedTests + failedTests) .. "\n")

  if failedTests > 0 then
    -- Print names of failed tests with test suite prefix
    print("\nFailed Tests:\n")

    for testName, result in pairs(testResults) do
      if not result.passed then
        print(testName)
      end
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

  Once all test suites are declared, call `runTests()` to
  execute all suites. This will call `os.exit` when finished
  with the exit code matching the number of failed tests.

  Example test suite:

  ```lua
    local TestFramework = require("lib.TestFramework")

    local describe = TestFramework.describe
    local beforeEach = TestFramework.beforeEach
    local beforeAll = TestFramework.beforeAll
    local should = TestFramework.should

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

      when("a when thing", function()
        _then("my test", function()
          assert(true, "Never gonna happen")
        end)

        and("another thing fail", function ()
          _then("my test", function()
            assert(false, "Always gonna happen")
          end)
        end)
      end)
    end)
  ```

]]
---@class TestFramework
---@field describe fun(name: string, func: fun())
---@field when fun(name: string, func: fun())
---@field _and fun(name: string, func: fun())
---@field beforeAll fun(func: fun())
---@field beforeEach fun(func: fun())
---@field should fun(name: string, func: fun())
---@field _then fun(name: string, func: fun())
---@field is fun(name: string, func: fun())
---@field runTests fun()

---@type TestFramework
local TestFramework = {
  describe = describe,
  when = when,
  _and = _and,
  _then = _then,
  beforeAll = beforeAll,
  beforeEach = beforeEach,
  should = should,
  is = is,
  runTests = runTests
}

return TestFramework
