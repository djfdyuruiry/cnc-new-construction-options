local ApiModule = require("nco.lib.ApiModule")
local TypeValidator = require("nco.lib.TypeValidator")
local Logger = require("nco.Logger")

local isNotBlank = TypeValidator.Validators.isNotBlank
local isType = TypeValidator.Validators.isType

--[[
  API for registering event callbacks.

  - Callbacks are currently called on scenario trigger execute
  - Trigger definition in scenario INI file will setup the
    execution when a certain condition is true
  - To define a callback (assuming scenario INI triggers declared):
    ```lua
      -- Event.X - X can be any valid name
      Event.handlers.onTimerTrigger = function(triggerName)
        Logger.debug("Handling trigger %s", triggerName)

        if triggerName == "TIME5" then
          Logger.debug("5 second timer execute")
        elseif triggerName == "TIME10" then
          Logger.debug("10 second timer execute")          
        end
      end
    ```
]]
---@class Event : ApiModule
---@field handlers { [string]: TriggerEventHandler }

---@alias TriggerEventHandler fun(triggerName: string)

---@param cppApi CppApiInstance
---@return Event
local function builder(cppApi)
  local eventHandlers = {}

  return {
    handlers = setmetatable(
      {},
      {
        __index = function(_, callbackName)
          local callbackFunc = eventHandlers[callbackName]

          if type(callbackFunc) ~= "function" then
            Logger.warning("Attempt to lookup undefined event handler: %s", callbackName)
          end

          return callbackFunc
        end,
        __newindex = function (_, callbackName, callbackFunc)
          TypeValidator.validateCall("Events.handlers.X", {
            callbackName = {callbackName, isType("string"), isNotBlank},
            callbackFunc = {callbackFunc, isType("function")}
          })

          Logger.info("Lua event handler registered: %s", callbackName)

          eventHandlers[callbackName] = callbackFunc
        end
      }
    )
  }
end

---@type Event
_G.Event = ApiModule({
  name = "Event",
  cppSource = "common/lua/event_luaapi.h",
  builder = builder
})

return _G.Event
