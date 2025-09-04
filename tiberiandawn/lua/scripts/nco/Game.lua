_G.TiberianDawn = _G.TiberianDawn and _G.TiberianDawn or {}

-- See: tiberiandawn/lua/game_luaapi.h
_G.TiberianDawn.Game = _G.TiberianDawn.Game and _G.TiberianDawn.Game or {
  __cpp_source = __CNC_API.Game.__cpp_source,
  __name = __CNC_API.Game.__name,

  playerWin = __CNC_API.Game.win,
  playerLose = __CNC_API.Game.lose
}
