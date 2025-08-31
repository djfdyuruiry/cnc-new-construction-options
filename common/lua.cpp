#include "lua.h"

/**
 * Static functions
 */
static lua_State* Build_State() {
  auto L = luaL_newstate();
  luaL_openlibs(L);

  return L;
}

/**
 * class LuaEngine
 */
const LuaEngine LuaEngine::Global = LuaEngine();

LuaEngine::LuaEngine() {
  Runtime = std::shared_ptr<lua_State>(Build_State(), lua_close);
}

void LuaEngine::With_State(std::function<void(lua_State*)> actions) {
  actions(Runtime.get());
}

bool LuaEngine::Exec(std::string_view script) {
  return luaL_dostring(Runtime.get(), script.data());
}

bool LuaEngine::Exec_File(std::string_view script) {
  return luaL_dofile(Runtime.get(), script.data());
}

luabridge::Namespace LuaEngine::Bridge() {
  return luabridge::getGlobalNamespace(Runtime.get());
}
