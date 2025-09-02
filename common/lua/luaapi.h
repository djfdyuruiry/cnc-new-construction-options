#pragma once

#include <filesystem>
#include <functional>
#include <string>

#include "logger.h"
#include "lua/luaengine.h"
#include "../paths.h"

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

  luabridge::Namespace Get_Namespace() const {
    return Lua.Bridge().beginNamespace(Name.data());
  }

  virtual void Register_Consts() const {};
  virtual void Register_Functions() const {};
  virtual void Register_Scripts() const {
    if (Scripts.size() < 1) {
      CNC_LOGGER_DEBUG("No scripts registered for this API");
      return;
    }

    CNC_LOGGER_INFO("Registering scripts using base path: {}", Lua_Directory.string());

    for (const auto& script : Scripts) {
      auto full_script_path = Lua_Directory / script;

      Lua.Exec_File(full_script_path.string())
        .If_Ok([&](auto& r) {
          CNC_LOGGER_INFO("Loaded script OK: {}", script.string());
        })
        .On_Error([&](auto& r) {
          CNC_LOGGER_ERROR("Failed to load script '{}': {}", script.string(), r.Error.value());
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
  inline static const std::filesystem::path Lua_Directory = std::filesystem::path(Paths.Program_Path()) / "lua";

  const LuaEngine& Lua;
  const std::string_view Name;
  const std::vector<std::filesystem::path> Scripts;
};
