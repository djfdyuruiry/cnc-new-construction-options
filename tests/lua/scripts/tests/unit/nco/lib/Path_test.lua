local TestFramework = require("lib.TestFramework")

local _then = TestFramework._then
local describe = TestFramework.describe
local beforeEach = TestFramework.beforeEach
local should = TestFramework.should
local when = TestFramework.when

local Path = require("nco.lib.Path")

describe("Path", function ()
  ---@type Path
  local testPath

  beforeEach(function ()
    testPath = Path("/usr/bin", "/", false)
  end)

  describe("join", function()
    when("rhs is string", function()
      _then("result path is correct", function()
        local result = testPath.join("lua")

        -- test __tostring
        assert(tostring(result) == "/usr/bin/lua")
      end)
    end)

    when("rhs is Path instance", function()
      _then("result path is correct", function()
        local result = testPath.join(Path("lua", "/", false))

        -- test __eq
        assert(result == Path("/usr/bin/lua", "/", false))
      end)
    end)
  end)

  describe("/ operator", function()
    when("rhs is string", function()
      _then("result path is correct", function()
        local result = testPath / "lua"

        -- test __eq
        assert(result == Path("/usr/bin/lua", "/", false))
      end)
    end)

    when("rhs is Path instance", function()
      _then("result path is correct", function()
        local result = testPath / Path("lua", "/", false)

        -- test __tostring
        assert(tostring(result) == "/usr/bin/lua")
      end)
    end)
  end)

  describe("isRelative", function()
    when("arg is relative path", function()
      _then("it should return true", function()
        local relativePath = Path("relative/path", "/", false)
        assert(relativePath.isRelative() == true)
      end)
    end)

    when("arg is absolute path", function()
      _then("it should return false", function()
        local absolutePath = Path("/absolute/path", "/", false)
        assert(absolutePath.isRelative() == false)
      end)
    end)
  end)

  describe("isSubPathOf", function()
    when("path is not a sub path", function()
      _then("it should return false", function()
        local childPath = Path("/other/path", "/", false)

        assert(childPath.isSubPathOf(testPath) == false)
      end)
    end)

    when("path is a sub path", function()
      _then("it should return true", function()
        local childPath = Path("/usr/bin/lua", "/", false)

        assert(childPath.isSubPathOf(testPath) == true)
      end)
    end)
  end)

  describe("asRelativeSubPathOf", function()
    when("path is not an ancestor of testPath", function()
      _then("it should throw an error", function()
        local path = Path("/other/path", "/", false)
        local result = pcall(path.asRelativeSubPathOf, testPath)

        assert(not result)
      end)
    end)

    when("path is an ancestor of testPath", function()
      _then("it should return the relative path", function()
        local path = Path("/usr/bin/env/local", "/", false)
        local result = path.asRelativeSubPathOf(testPath)

        -- test __tostring
        assert(tostring(result) == "env/local")
      end)
    end)
  end)

  describe("getRoot", function()
    should("get first path for relative path", function()
      local relativePath = Path("relative/path", "/", false)
      local result = relativePath.getRoot()

      -- test __tostring
      assert(tostring(result) == "relative")
    end)

    should("get system root path for absolute path", function()
      local result = testPath.getRoot()

      -- test __eq
      assert(result == Path("/", "/", false))
    end)
  end)
end)
