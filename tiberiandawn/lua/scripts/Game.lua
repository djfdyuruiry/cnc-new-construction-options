_G.TiberianDawn = _G.TiberianDawn and _G.TiberianDawn or {
  Game = {},
  Messages = {},
  UI = {}
}

_G.TiberianDawn.Game.playerWin = function(...)
  -- See: tiberiandawn/lua/game_luaapi.h
  __CNC_API.Game.win(...);
end

_G.TiberianDawn.Game.playerLose = function(...)
  -- See: tiberiandawn/lua/game_luaapi.h
  __CNC_API.Game.lose(...);
end
