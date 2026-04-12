local ApiModule = require("nco.lib.ApiModule")
local Path = require("nco.lib.Path")

--[[
  API to work with file paths and provide OS info.

  - Returns file objects similar to io.open
  - openGameFile/openUserFile functions return file object and optional error string
  - openGameFile will access files stored beside the game binary (exe on Windows/dll if running as a Remastered mod)
  - openUserFile will access files stored in the users config directory (ini files etc.)

  See: nco.lib.Path
]]
---@class System : ApiModule
---@field pathSeparator string
---@field isMacOS boolean
---@field isUnix boolean
---@field isWindows boolean
---@field isRemasteredMod boolean
---@field gamePath Path
---@field userPath Path
---@field openGameFile fun(subPath: Path|string, mode?: openmode): file*, string?
---@field openUserFile fun(subPath: Path|string, mode?: openmode): file*, string?
---@field Path fun(path: Path|string): Path

---@param cppApi CppApiInstance
---@return System
local function builder(cppApi)
  local system = {
    pathSeparator = cppApi.pathSeparator,
    isMacOS = cppApi.isMacOS,
    isUnix = cppApi.isUnix,
    isWindows = cppApi.isWindows,
    isRemasteredMod = cppApi.isRemasteredMod,

    ---@param rootPath Path|string
    ---@param subPath string
    ---@param mode openmode?
    _openFile = function(rootPath, subPath, mode)
      local fullPath = Path(rootPath, cppApi.pathSeparator, cppApi.isWindows) / subPath

      if type(mode) ~= "string" then
        mode = "r"
      end

      return io.open(tostring(fullPath), mode)
    end,

    -- Wrapper around Path class that passes required system params
    ---@param pathStringOrPath Path|string
    ---@return Path
    Path = function(pathStringOrPath)
      return Path(pathStringOrPath, cppApi.pathSeparator, cppApi.isWindows)
    end
  }

  -- make path objects and aliases to _openFile
  for _, pathField in ipairs({ "game", "user"}) do
    local upperName = pathField:sub(1, 1):upper() .. pathField:sub(2)
    local funcName = string.format("open%sFile", upperName)

    local fieldName = string.format("%sPath", pathField)

    ---@diagnostic disable-next-line: assign-type-mismatch
    system[fieldName] = system.Path(cppApi[fieldName])

    system[funcName] = function(...)
      return system._openFile(system[fieldName], ...)
    end
  end

  return system
end

---@type System
_G.System = ApiModule({
  name = "System",
  cppSource = "common/lua/system_luaapi.h",
  builder = builder
})

return _G.System
