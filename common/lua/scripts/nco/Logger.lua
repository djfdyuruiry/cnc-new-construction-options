local ApiModule = require("nco.lib.ApiModule")
local System = require("nco.System")

---@alias LogLevel "trace"|"debug"|"info"|"warning"|"error"|"critical"

--[[
  API that writes to game engine logs. You can view these logs by opening the NCO log file.
  See: https://github.com/djfdyuruiry/cnc-new-construction-options/wiki/3b.Lua-Advanced-Guide#logging

  - Methods will capture the call stack of the caller automatically
  - Methods support printf style formatting for ease of use:
    ```lua
      Logger.error("Something bad happened. %s", errorMessage)
    ```
  - You can change the current log level to write debug logs etc.
    ```lua
      Logger.setLevel("debug")

      Logger.debug("This debug message will be written to logs")
    ```
]]
---@class Logger : ApiModule
---@field getLevel fun(): LogLevel
---@field setLevel fun(level: LogLevel)
---@field log fun(level: LogLevel, message:string, ...)
---@field trace fun(message:string, ...)
---@field debug fun(message:string, ...)
---@field info fun(message:string, ...)
---@field warning fun(message:string, ...)
---@field error fun(message:string, ...)
---@field critical fun(message:string, ...)

---@param cppApi CppApiInstance
---@return Logger
local function builder(cppApi)
  local logger = {
    getLevel = cppApi.getLevel,
    setLevel = cppApi.setLevel,

    ---@param level LogLevel
    ---@param message string
    log = function(level, message, ...)
      local caller = debug.getinfo(3)
      local callerSource = caller.source

      if callerSource:match("^@") then
        -- source is a file
        local sourceFilePath = System.Path(callerSource:match("^@(.+)$"))

        if sourceFilePath.isSubPathOf(System.gamePath) then
          -- remove leading path if it's a file inside the game Lua directory
          callerSource = tostring(
            sourceFilePath.asRelativeSubPathOf(System.gamePath)
          )
        end

        if sourceFilePath.isSubPathOf(System.userPath) then
          -- remove leading path if it's a file inside the user Lua directory
          callerSource = tostring(
            sourceFilePath.asRelativeSubPathOf(System.userPath)
          )
        end
      end

      local sourceLocation = string.format(
        "%s:%d %s()",
        callerSource,
        caller.currentline,
        caller.name
      )

      cppApi.log(sourceLocation, level, string.format(tostring(message), ...))
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
_G.Logger = ApiModule({
  name = "Logger",
  cppSource = "common/lua/logging_luaapi.h",
  builder = builder
})

return _G.Logger
