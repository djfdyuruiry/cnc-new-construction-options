_G.TiberianDawn = _G.TiberianDawn and _G.TiberianDawn or {
  Game = {},
  Messages = {},
  UI = {}
}

_G.TiberianDawn.Messages.showToPlayer = function(...)
  -- See: tiberiandawn/lua/game_luaapi.h
  __CNC_API.Messages.showToPlayer(...);
end
