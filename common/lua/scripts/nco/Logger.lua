-- See: common/lua/logging_luaapi.h
-- Requires: common/lua/system_luaapi.h
_G.Logger = _G.Logger and _G.Logger or {
  level = __CNC_API.Logger.level,

  --[[
    Wrapper around LoggerLuaApi that passes current
    lua source location and log level/message.

    Supports printf style formatting.
  ]]
  _log = function(level, message, ...)
    local caller = debug.getinfo(3)
    local callerSource = caller.source

    if callerSource:match("^@") then
      -- source is a file
      local sourceFilePath = System.Path(callerSource:match("^@(.+)$"))

      if sourceFilePath.isSubPathOf(System.luaPath) then
        -- remove leading path if it's a file inside the standard Lua directory
        callerSource = sourceFilePath.asRelativeSubPathOf(System.luaPath)
      end
    end

    local source_location = string.format(
      "%s:%d %s()",
      callerSource,
      caller.currentline,
      caller.name
    )

    __CNC_API.Logger.log(source_location, level, string.format(tostring(message), ...))
  end
}

-- Make level aliases that call _log 
for _, v in pairs({ "trace", "debug", "info", "warning", "error", "critical" }) do
  _G.Logger[v] = function(message, ...)
    _G.Logger._log(v, message, ...)
  end
end
