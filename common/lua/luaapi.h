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
 *   local var = <Root>.<Name>.<const>
 *   local result = <Root>.<Name>.<function>(...)
 *
 * Where <Root> is managed by @property{LuaEngine::Root_Api_Namespace}.
 * 
 * Lua scripts can be loaded from disk by passing a 'scripts' vector
 * in the constructor. These scripts are expected to be in "<LUA_DIR>/nco"
 * , where <LUA_DIR> is managed by @property{LuaEngine::Lua_Path}.
 */
class LuaApi
{
public:
    inline static const std::filesystem::path Nco_Directory = "nco";

    const std::string_view Name;

    LuaApi(const std::string_view name, const std::vector<std::filesystem::path> scripts)
        : Name(name), Scripts(scripts) {}

    LuaApi(const std::string_view name)
        : Name(name), Scripts() {}

    void With_Api_Namespace(LuaEngine& engine, std::function<void(luabridge::Namespace&)> action) const
    {
        engine.With_Api_Namespace(Name, action);
    }

    void Register_Api_Metadata(LuaEngine& engine) const
    {
        With_Api_Namespace(engine, [&](auto& n) {
            n.addConstant("__cppSource", Get_Cpp_Source())
             .addConstant("__name", Name.data());
        });
    }

    /**
     * Ensure the host Lua engine has other APIs
     * that are a dependency of this API here.
     */
    virtual void Register_Dependencies(LuaEngine& engine) const {}

    virtual void Register_Consts(LuaEngine& engine) const {}

    virtual void Register_Functions(LuaEngine& engine) const {}

    virtual void Register_Scripts(LuaEngine& engine) const
    {
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

            engine.Exec_File(full_script_path.string())
                .If_Ok([&](auto& r) { CNC_LOGGER_INFO("Loaded script OK: {}", script.string()); })
                .On_Error([&](auto& r) {
                    CNC_LOGGER_FATAL("Failed to load script '{}': {}", script.string(), r.Error.value());
                });
        }
    }

    virtual void Register(LuaEngine& engine)
    {
        CNC_LOGGER_INFO("Registering Lua API: {}", Name);

        Register_Api_Metadata(engine);

        Register_Dependencies(engine);

        Register_Consts(engine);
        Register_Functions(engine);
        Register_Scripts(engine);
    }

protected:
    inline static const CncLogger Logger = CncLogger("LuaApi");

    const std::vector<std::filesystem::path> Scripts;

    /**
     * Sub-classes should copy/paste this with 'override'
     * to ensure source file is correct.
     */
    virtual const char* Get_Cpp_Source() const
    {
        return __FILE__;
    }
};
