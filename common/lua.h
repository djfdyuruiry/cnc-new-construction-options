#pragma once

#include <functional>
#include <memory>

#include <lua.hpp>

class LuaEngine {
public:
  static const LuaEngine Instance;

  LuaEngine();

  void With_State(std::function<void(lua_State*)> actions);
private:
  std::shared_ptr<lua_State> Runtime;
};
