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

  LuaApi(
    const LuaEngine& engine,
    const std::string_view name,
    const std::vector<std::filesystem::path> scripts
  ) :
    Lua(engine),
    Name(name),
    Scripts(scripts) {}

  LuaApi(
    const LuaEngine& engine,
    const std::string_view name
  ) :
    Lua(engine),
    Name(name),
    Scripts() {}

  luabridge::Namespace Get_Api_Namespace() const {
    return Lua.Bridge()
      .beginNamespace(Root_Namespace.data())
      .beginNamespace(Name.data());
  }

  void With_Api_Namespace(std::function<void(luabridge::Namespace&)> action) const {
    auto api_namespace = Get_Api_Namespace();

    action(api_namespace);

    api_namespace.endNamespace().endNamespace();
  }

  virtual void Register_Consts() const {};
  virtual void Register_Functions() const {};
  virtual void Register_Scripts() const {
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

  virtual void Register() {
    CNC_LOGGER_INFO("Registering Lua API: {}", Name);

    Register_Consts();
    Register_Functions();
    Register_Scripts();
  }
protected:
  inline static const CncLogger Logger = CncLogger("LuaApi");
  inline static const std::string_view Root_Namespace = "__CNC_API";

  const LuaEngine& Lua;
  const std::string_view Name;
  const std::vector<std::filesystem::path> Scripts;
};
