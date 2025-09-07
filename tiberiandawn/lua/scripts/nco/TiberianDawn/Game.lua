local TdApiModule = require("nco.TiberianDawn.lib.TdApiModule")

--[[
  API that controls the Tiberian Dawn game engine.
]]
---@class Game : ApiModule
---@field win fun()
---@field lose fun()

---@type Game
_G.Game = TdApiModule({
  name = "Game",
  cppSource = "tiberiandawn/lua/game_luaapi.h",
  builder = function(cppApi)
    return {
      playerWin = cppApi.win,
      playerLose = cppApi.lose
    }
  end
})

return _G.Game
