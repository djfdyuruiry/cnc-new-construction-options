local ApiModule = require("nco.lib.ApiModule")

--[[
  API that controls the Tiberian Dawn game engine.
]]
---@class Game : ApiModule
---@field win fun()
---@field lose fun()

---@type Game
local Game = ApiModule({
  modulePath = { "TiberianDawn", "Game" },
  cppApi = "Game",
  cppSource = "tiberiandawn/lua/game_luaapi.h",
  builder = function(cppApi)
    return {
      playerWin = cppApi.win,
      playerLose = cppApi.lose
    }
  end
})

return Game
