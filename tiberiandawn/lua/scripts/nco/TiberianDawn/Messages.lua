local TdApiModule = require("nco.TiberianDawn.lib.TdApiModule")

--[[
  API for showing in-game messages to the player.
]]
---@class Messages : ApiModule
---@field sendToPlayer fun(message: string, ...) Supports string.format style calls

---@type Messages
_G.Messages = TdApiModule({
  name = "Messages",
  cppSource = "tiberiandawn/lua/messages_luaapi.h",
  builder = function(cppApi)
    return {
      sendToPlayer = function(message, ...)
        cppApi.sendToPlayer(
          string.format(tostring(message), ...)
        )
      end
    }
  end
})

return _G.Messages
