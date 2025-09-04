local Path = require("nco.lib.Path")

-- See: common/lua/system_luaapi.h
_G.System = _G.System and _G.System or {
  __cpp_source = __CNC_API.System.__cpp_source,
  __name = __CNC_API.System.__name,

  pathSeparator = __CNC_API.System.pathSeparator,
  isWindows = __CNC_API.System.isWindows,

  _openFile = function(rootPath, subPath, mode)
    local fullPath = _G.System.Path(rootPath) / subPath

    return io.open(fullPath, mode)
  end,

  -- Wrapper around Path class that passes required system params
  Path = function(pathStringOrPath)
    return Path(pathStringOrPath, __CNC_API.System.pathSeparator, __CNC_API.System.isWindows)
  end
}

-- make path objects and aliases to _openFile
for _, pathField in ipairs({ "gamePath", "luaPath", "userPath"}) do
  local upperName = pathField:sub(1, 1):upper() .. pathField:sub(2)
  local pathName = upperName:gmatch("[a-z]+")()

  local funcName = string.format("open%sFile", pathName)

  ---@diagnostic disable-next-line: assign-type-mismatch
  _G.System[pathField] = _G.System.Path(__CNC_API.System[pathField])

  _G.System[funcName] = function(...)
    _G.System._openFile(_G.System[pathField], ...)
  end
end
