#include "lua.h"

const LuaEngine LuaEngine::Instance = LuaEngine();

LuaEngine::LuaEngine() {
  Runtime = std::make_shared<lua_State>(luaL_newstate, lua_close);
}

void LuaEngine::With_State(std::function<void(lua_State*)> actions) {
  actions(Runtime.get());
}
