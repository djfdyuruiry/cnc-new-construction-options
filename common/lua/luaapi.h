#pragma once

#include <functional>
#include <string>

#include "logger.h"
#include "luaengine.h"

/**
 * Wrapper around LuaEngine to register Lua
 * APIs as global tables:
 *
 *   local var = <Name>.<const>
 *   local result = <Name>.<function>()
 *
 */
class LuaApi {
public:
  LuaApi(const LuaEngine& engine, const std::string_view name) : Lua(engine), Name(name) {}

  luabridge::Namespace Get_Namespace() const {
    return Lua.Bridge().beginNamespace(Name.data());
  }

  virtual void Register_Consts() const {};
  virtual void Register_Functions() const {};

  virtual void Register() {
    CNC_LOGGER_INFO("Registering Lua API: {}", Name);

    Register_Consts();
    Register_Functions();
  }
protected:
  inline static CncLogger Logger = CncLogger("LuaApi");

  const std::string_view Name;
  const LuaEngine& Lua;
};
