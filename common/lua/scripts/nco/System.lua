local ApiModule = require("nco.lib.ApiModule")
local Path = require("nco.lib.Path")

local function builder(cppApi)
  local system = {
    pathSeparator = cppApi.pathSeparator,
    isWindows = cppApi.isWindows,

    _openFile = function(rootPath, subPath, mode)
      local fullPath = Path(rootPath, cppApi.pathSeparator, cppApi.isWindows) / subPath

      return io.open(fullPath, mode)
    end,

    -- Wrapper around Path class that passes required system params
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

return ApiModule({
  modulePath = {"System"},
  cppApi = "System",
  cppSource = "common/lua/system_luaapi.h",
  builder = builder
})
