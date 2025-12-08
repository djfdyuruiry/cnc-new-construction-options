#include <filesystem>
#include <functional>
#include <string>

#include "logger.h"

#include "luaapi.h"
#include "luaengine.h"
#include "luaarguments.h"

// Method implementations
void LuaApi::With_Api_Namespace(const LuaEngine& engine, std::function<void(luabridge::Namespace&)> action) const
{
    engine.With_Api_Namespace(Name, std::move(action));
}

void LuaApi::Register_Api_Metadata(const LuaEngine& engine) const
{
    With_Api_Namespace(engine, [&](auto& n) {
        n.addConstant("__cppSource", Get_Cpp_Source())
         .addConstant("__name", Name.data());
    });
}

void LuaApi::Register_Native_Module(LuaEngine& engine) const
{
    if (!Has_Native_Module) {
        return;
    }

    auto module_path = std::format("{}.{}", Get_Parent_Lua_Module_Path(), Name);

    CNC_LOGGER_DEBUG("Registering native module: {}", module_path);

    engine.With_Global(Require_Global, LUA_TFUNCTION, [&]() {
        return engine.PCall_With_Args(Require_Global, module_path)
            .On_Error([&](auto& r) {
                CNC_LOGGER_FATAL(
                    "Failed to register native module '{}': {}",
                    module_path,
                    r.Error_Message()
                );
            });
    });
}

void LuaApi::Register_Scripts(LuaEngine& engine) const
{
    if (Scripts.empty()) {
        CNC_LOGGER_DEBUG("No scripts registered for this API");
        return;
    }

    CNC_LOGGER_INFO("Registering scripts using base path: {}", LuaEngine::Get_Lua_Path().string());

    for (const auto& script : Scripts) {
        auto full_script_path = script;

        if (script.is_relative()) {
            // assume relative paths are part of the 'nco' library
            full_script_path = Nco_Directory / script;
        }

        engine.Exec_File(full_script_path.string())
            .If_Ok([&](auto& r) { CNC_LOGGER_INFO("Loaded script OK: {}", script.string()); })
            .On_Error([&](auto& r) {
                CNC_LOGGER_FATAL(
                    "Failed to load script '{}': {}",
                    script.string(),
                    r.Error_Message()
                );
            });
    }
}

void LuaApi::Register(LuaEngine& engine) const
{
    CNC_LOGGER_INFO("Registering Lua API: {}", Name);

    Register_Dependencies(engine);

    // c++ code
    Register_Api_Metadata(engine);
    Register_Consts(engine);
    Register_Functions(engine);

    // lua code
    Register_Native_Module(engine);
    Register_Scripts(engine);
}

void LuaApi::Register_Dependencies(LuaEngine& engine) const
{
    // Base implementation - does nothing
}

void LuaApi::Register_Consts(LuaEngine& engine) const
{
    // Base implementation - does nothing
}

void LuaApi::Register_Functions(LuaEngine& engine) const
{
    // Base implementation - does nothing
}

// Implementation for Get_Parent_Lua_Module_Path
const std::string& LuaApi::Get_Parent_Lua_Module_Path() const
{
    static const auto module = Nco_Directory.string();
    return module;
}
