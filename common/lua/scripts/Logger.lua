_G.Logger = _G.Logger and _G.Logger or {}

--[[
  Wrapper around LoggerLuaApi that passes current
  lua source location and log level/message.

  Supports printf style formatting.
]]
_G.Logger._log = function(level, message, ...)
  local caller = debug.getinfo(3)

  local source_location = string.format(
    "%s:%d %s()",
    caller.short_src,
    caller.currentline,
    caller.name
  )

  -- See: common/lua/logging_luaapi.h
  __CNC_API.Logger.log(source_location, level, string.format(message, ...))
end

_G.Logger.level = __CNC_API.Logger.level

-- Make level aliases that call _log 
for _, v in pairs({ "trace", "debug", "info", "warning", "error", "critical" }) do
  _G.Logger[v] = function(message, ...)
    _G.Logger._log(v, message, ...)
  end
end
