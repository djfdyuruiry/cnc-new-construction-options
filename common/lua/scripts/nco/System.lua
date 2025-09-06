local ApiModule = require("nco.lib.ApiModule")
local Path = require("nco.lib.Path")

--[[
  API to work with file paths and provide OS info.

  See: nco.lib.Path
]]
---@class System : ApiModule
---@field pathSeparator string
---@field isWindows boolean
---@field gamePath Path
---@field luaPath Path
---@field userPath Path
---@field openGameFile fun(subPath: Path|string, mode?: openmode): file*, string?
---@field openLuaFile fun(subPath: Path|string, mode?: openmode): file*, string?
---@field openUserFile fun(subPath: Path|string, mode?: openmode): file*, string?
---@field Path fun(path: Path|string): Path

---@return System
local function builder(cppApi)
  local system = {
    pathSeparator = cppApi.pathSeparator,
    isWindows = cppApi.isWindows,

    ---@param rootPath Path|string
    ---@param subPath string
    ---@param mode openmode
    _openFile = function(rootPath, subPath, mode)
      local fullPath = Path(rootPath, cppApi.pathSeparator, cppApi.isWindows) / subPath

      return io.open(fullPath, mode)
    end,

    -- Wrapper around Path class that passes required system params
    ---@param pathStringOrPath Path|string
    ---@return Path
    Path = function(pathStringOrPath)
      return Path(pathStringOrPath, cppApi.pathSeparator, cppApi.isWindows)
    end
  }

  -- make path objects and aliases to _openFile
  for _, pathField in ipairs({ "gamePath", "luaPath", "userPath"}) do
    local upperName = pathField:sub(1, 1):upper() .. pathField:sub(2)
    local pathName = upperName:gmatch("[a-z]+")()

    local funcName = string.format("open%sFile", pathName)

    ---@diagnostic disable-next-line: assign-type-mismatch
    system[pathField] = system.Path(cppApi[pathField])

    system[funcName] = function(...)
      system._openFile(system[pathField], ...)
    end
  end

  return system
end

---@type System
local System = ApiModule({
  modulePath = {"System"},
  cppApi = "System",
  cppSource = "common/lua/system_luaapi.h",
  builder = builder
})

return System
