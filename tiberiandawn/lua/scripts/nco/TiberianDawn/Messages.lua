local ApiModule = require("nco.lib.ApiModule")

--[[
  API for showing in-game messages to the player.
]]
---@class Messages : ApiModule
---@field showToPlayer fun(message: string, ...) Supports string.format style calls

---@type Messages
local Messages = ApiModule({
  modulePath = { "TiberianDawn", "Messages" },
  cppApi = "Messages",
  cppSource = "tiberiandawn/lua/messages_luaapi.h",
  builder = function(cppApi)
    return {
      showToPlayer = function(message, ...)
        cppApi.showToPlayer(
          string.format(message, ...)
        )
      end
    }
  end
})

return Messages
