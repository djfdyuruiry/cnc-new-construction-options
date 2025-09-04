if type(__CNC_API) == "nil" or (__CNC_API.Game) == "nil" then
  error("nco.TiberianDawn.Game failed to init, required C++ backend not loaded: tiberiandawn/lua/game_luaapi.h")
end

_G.TiberianDawn = _G.TiberianDawn and _G.TiberianDawn or {}

_G.TiberianDawn.Game = _G.TiberianDawn.Game and _G.TiberianDawn.Game or {
  __cpp_source = __CNC_API.Game.__cpp_source,
  __name = __CNC_API.Game.__name,

  playerWin = __CNC_API.Game.win,
  playerLose = __CNC_API.Game.lose
}

return _G.TiberianDawn.Game
