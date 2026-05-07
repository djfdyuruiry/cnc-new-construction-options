local TypeValidator = require("nco.lib.TypeValidator")

local TdApiModule = require("nco.TiberianDawn.lib.TdApiModule")

local isOneOf = TypeValidator.Validators.isOneOf

-- TODO: add all colours
---@alias Colour "GREEN" | "RED" | "BLUE"

--[[
  API for showing in-game messages to the player.
]]
---@class Messages : ApiModule
---@field sendToPlayer fun(message: string, ...) Supports string.format style calls
---@field setColour fun(colour: Colour) Set the display colour for player messages
---@field resetColour fun() Reset the display colour for player messages to the default (Green)

---@type Messages
_G.Messages = TdApiModule({
  name = "Messages",
  cppSource = "tiberiandawn/lua/messages_luaapi.h",
  builder = function(cppApi)
    local defaultColour = "GREEN"
    local messageColour = defaultColour

    return {
      resetColour = function()
         messageColour = defaultColour
      end,
      sendToPlayer = function(message, ...)
        cppApi.sendToPlayer(
          string.format(tostring(message), ...),
          messageColour
        )
      end,
      setColour = function(colour)
        TypeValidator.validateCall("setColour", {
          colour = {colour, isOneOf({ "GREEN", "RED", "BLUE" })}
        })

        messageColour = colour
      end
    }
  end
})

return _G.Messages
