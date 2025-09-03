#pragma once

#include <filesystem>
#include <functional>
#include <string>

#include "../logger.h"
#include "../paths.h"

#include "luaengine.h"

/**
 * Wrapper around LuaEngine to register Lua
 * APIs as global tables:
 *
 *   local var = <Name>.<const>
 *   local result = <Name>.<function>()
 *
 * Lua scripts can be loaded from disk by passing a 'scripts' vector
 * in the constructor. These scripts are expected to be in "<LUA_DIR>/nco"
 * , where <LUA_DIR> is managed by @class{LuaEngine}.
 */
class LuaApi {
public:
  inline static const std::filesystem::path Nco_Directory = "nco";

  const std::string_view Name;

  LuaApi(const std::string_view name, const std::vector<std::filesystem::path> scripts) : Name(name), Scripts(scripts) {}
  LuaApi(const std::string_view name) : Name(name), Scripts() {}

  void With_Api_Namespace(LuaEngine& engine, std::function<void(luabridge::Namespace&)> action) const {
    engine.With_Api_Namespace(Name, action);
  }

  /**
   * Ensure the host Lua engine has other APIs
   * that are a dependency of this API here.
   */
  virtual void Register_Dependencies(LuaEngine& Lua) const {}

  virtual void Register_Consts(LuaEngine& Lua) const {};

  virtual void Register_Functions(LuaEngine& Lua) const {};

  virtual void Register_Scripts(LuaEngine& Lua) const {
    if (Scripts.size() < 1) {
      CNC_LOGGER_DEBUG("No scripts registered for this API");
      return;
    }

    CNC_LOGGER_INFO("Registering scripts using base path: {}", LuaEngine::Lua_Path.string());

    for (const auto& script : Scripts) {
      auto full_script_path = script;

      if (script.is_relative()) {
        // assume relative paths are part of the 'nco' library
        full_script_path = Nco_Directory / script;
      }

      Lua.Exec_File(full_script_path.string())
        .If_Ok([&](auto& r) {
          CNC_LOGGER_INFO("Loaded script OK: {}", script.string());
        })
        .On_Error([&](auto& r) {
          CNC_LOGGER_FATAL("Failed to load script '{}': {}", script.string(), r.Error.value());
        });      
    }
  }

  virtual void Register(LuaEngine& Lua) {
    CNC_LOGGER_INFO("Registering Lua API: {}", Name);

    Register_Dependencies(Lua);
    Register_Consts(Lua);
    Register_Functions(Lua);
    Register_Scripts(Lua);
  }
protected:
  inline static const CncLogger Logger = CncLogger("LuaApi");

  const std::vector<std::filesystem::path> Scripts;
};
