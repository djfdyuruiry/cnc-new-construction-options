local TdApiModule = require("nco.TiberianDawn.lib.TdApiModule")

--[[
  API for showing in-game messages to the player.
]]
---@class Messages : ApiModule
---@field showToPlayer fun(message: string, ...) Supports string.format style calls

---@type Messages
_G.Messages = TdApiModule({
  name = "Messages",
  cppSource = "tiberiandawn/lua/messages_luaapi.h",
  builder = function(cppApi)
    return {
      showToPlayer = function(message, ...)
        cppApi.showToPlayer(
          string.format(tostring(message), ...)
        )
      end
    }
  end
})

return _G.Messages
