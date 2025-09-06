local ApiModule = require("nco.lib.ApiModule")
local System = require("nco.System")

---@alias LogLevel "trace"|"debug"|"info"|"warning"|"error"|"critical"

--[[
  API that writes to game engine logs.

  - Methods will capture the call stack of the caller automatically
  - Methods support printf style formatting for ease of use:
    ```lua
      Logger.error("Something bad happened. %s", errorMessage)
    ```
]]
---@class Logger 
---@field level LogLevel
---@field log fun(level: LogLevel, message:string, ...)
---@field trace fun(message:string, ...)
---@field debug fun(message:string, ...)
---@field info fun(message:string, ...)
---@field warning fun(message:string, ...)
---@field error fun(message:string, ...)
---@field critical fun(message:string, ...)

---@return Logger
local function builder(cppApi)
  local logger = {
    level = cppApi.level,

    ---@param level LogLevel
    ---@param message string
    log = function(level, message, ...)
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

  -- Make level aliases that call log (matches spdlog levels) 
  for _, v in pairs({ "trace", "debug", "info", "warning", "error", "critical" }) do
    logger[v] = function(message, ...)
      logger.log(v, message, ...)
    end
  end

  return logger
end

---@type Logger
local Logger = ApiModule({
  modulePath = {"Logger"},
  cppApi = "Logger",
  cppSource = "common/lua/logging_luaapi.h",
  builder = builder
})

return Logger
