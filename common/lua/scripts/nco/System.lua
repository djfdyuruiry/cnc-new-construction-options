local Path = require("nco.lib.Path")

-- See: common/lua/system_luaapi.h
_G.System = _G.System and _G.System or {
  pathSeparator = __CNC_API.System.pathSeparator,
  isWindows = __CNC_API.System.isWindows,

  _openFile = function(rootPath, subPath, mode)
    local fullPath = Path(rootPath) / subPath

    return io.open(fullPath, mode)
  end
}

-- make path objects andaliases to _openFile
for _, pathField in ipairs({ "gamePath", "luaPath", "userPath"}) do
  local upperName = pathField:sub(1, 1):upper() .. pathField:sub(2)
  local pathName = upperName:gmatch("[a-z]+")()

  local funcName = string.format("open%sFile", pathName)

  _G.System[pathField] = Path(
    __CNC_API.System[pathField],
    __CNC_API.System.pathSeparator,
    __CNC_API.System.isWindows
  )

  _G.System[funcName] = function(...)
    _G.System._openFile(_G.System[pathField], ...)
  end
end
