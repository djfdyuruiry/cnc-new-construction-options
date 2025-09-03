_G.TiberianDawn = _G.TiberianDawn and _G.TiberianDawn or {}

-- See: tiberiandawn/lua/game_luaapi.h
_G.TiberianDawn.Game = _G.TiberianDawn.Game and _G.TiberianDawn.Game or {
  playerWin = __CNC_API.Game.win,
  playerLose = __CNC_API.Game.lose
}
