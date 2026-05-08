local TypeValidator = require("nco.lib.TypeValidator")

local TdApiModule = require("nco.TiberianDawn.lib.TdApiModule")

local isNotBlank = TypeValidator.Validators.isNotBlank
local isType = TypeValidator.Validators.isType

---@alias Colour "YELLOW" | "RED" | "CYAN" | "BLUE" | "ORANGE" | "GREEN" | "BROWN"

--[[
  API for showing in-game messages to the player.
]]
---@class Messages : ApiModule
---@field setColour fun(colour: Colour) Set the display colour for player messages
---@field resetColour fun() Reset the display colour for player messages to the default (Green)
---@field setMessageTimeout fun(seconds: number) Set timeout before message is removed from the screen
---@field resetMessageTimeout fun() Reset the timeout for messages (30 seconds)
---@field sendToPlayer fun(message: string, ...) Supports string.format style calls

---@type Messages
_G.Messages = TdApiModule({
  name = "Messages",
  cppSource = "tiberiandawn/lua/messages_luaapi.h",
  builder = function(cppApi)
    local defaultColour = "GREEN"
    local defaultMessageTimeoutInSecs = 30
    local messageColour = defaultColour
    local messageTimeoutInSecs = defaultMessageTimeoutInSecs

    return {
      resetColour = function()
         messageColour = defaultColour
      end,
      resetMessageTimeout = function()
         messageTimeoutInSecs = defaultMessageTimeoutInSecs
      end,
      sendToPlayer = function(message, ...)
        cppApi.sendToPlayer(
          string.format(tostring(message), ...),
          messageColour,
          messageTimeoutInSecs
        )
      end,
      setColour = function(colour)
        TypeValidator.validateCall("setColour", {
          colour = {colour, isType("string"), isNotBlank}
        })

        cppApi.validateColorType(colour);

        messageColour = colour
      end,
      setMessageTimeout = function(seconds)
        TypeValidator.validateCall("setColour", {
          seconds = {seconds, isType("number")}
        })

        if seconds < 1 then
          error("Messages.setMessageTimeout: argument 'seconds' must be greater than 0")
        end

        messageTimeoutInSecs = seconds
      end
    }
  end
})

return _G.Messages
