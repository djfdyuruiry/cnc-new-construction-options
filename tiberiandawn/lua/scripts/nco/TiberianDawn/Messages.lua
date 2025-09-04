if type(__CNC_API) == "nil" or (__CNC_API.Messages) == "nil" then
  error("nco.TiberianDawn.Messages failed to init, required C++ backend not loaded: tiberiandawn/lua/messages_luaapi.h")
end

_G.TiberianDawn = _G.TiberianDawn and _G.TiberianDawn or {}

_G.TiberianDawn.Messages = _G.TiberianDawn.Messages and _G.TiberianDawn.Messages or {
  __cpp_source = __CNC_API.Messages.__cpp_source,
  __name = __CNC_API.Messages.__name,

  showToPlayer = __CNC_API.Messages.showToPlayer
}

return _G.TiberianDawn.Messages
