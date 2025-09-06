local ApiModule = require("nco.lib.ApiModule")
local System = require("nco.System")

local function builder(cppApi)
  local logger = {
    level = cppApi.level,

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

      cppApi.log(source_location, level, string.format(tostring(message), ...))
    end
  }

  -- Make level aliases that call _log 
  for _, v in pairs({ "trace", "debug", "info", "warning", "error", "critical" }) do
    logger[v] = function(message, ...)
      logger._log(v, message, ...)
    end
  end

  return logger
end

---@type { 
---  level: string,
---  _log: fun(level:string, message:string, ...):nil,
---  trace:fun(message:string, ...):nil,
---  debug:fun(message:string, ...):nil,
---  info:fun(message:string, ...):nil,
---  warning:fun(message:string, ...):nil,
---  error:fun(message:string, ...):nil,
---  critical:fun(message:string, ...):nil}
local Logger = ApiModule({
  modulePath = {"Logger"},
  cppApi = "Logger",
  cppSource = "common/lua/logging_luaapi.h",
  builder = builder
})

return Logger
