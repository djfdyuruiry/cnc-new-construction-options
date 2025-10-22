#pragma once

#include <filesystem>
#include <functional>
#include <string>

#include "../logger.h"
#include "../paths.h"

#include "luaengine.h"
#include "luaarguments.h"

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
    static inline const std::filesystem::path Nco_Directory = "nco";
    static inline const std::string_view Require_Global = "require";

    const std::string_view Name;
    const bool Has_Native_Module;

    LuaApi(const std::string_view name)
        : Name(name), Scripts(), Has_Native_Module(true) {}

    LuaApi(const std::string_view name, const bool has_native_module)
        : Name(name), Scripts(), Has_Native_Module(has_native_module) {}

    LuaApi(const std::string_view name, const bool has_native_module, const std::vector<std::filesystem::path> scripts)
        : Name(name), Scripts(scripts), Has_Native_Module(has_native_module) {}

    virtual ~LuaApi() = default;

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
     * that are a dependency of this API.
     */
    virtual void Register_Dependencies(LuaEngine& engine) const {}

    virtual void Register_Consts(LuaEngine& engine) const {}

    virtual void Register_Functions(LuaEngine& engine) const {}

    virtual void Register_Native_Module(LuaEngine& engine) const
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

    virtual void Register_Scripts(LuaEngine& engine) const
    {
        if (Scripts.size() < 1) {
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

    virtual void Register(LuaEngine& engine) const
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

protected:
    static inline const auto& Logger = CncLogger::For(LuaApi);

    std::vector<std::filesystem::path> Scripts;

    static std::function<int(lua_State*)> Void_Lua_Function(std::function<void(LuaEngine&)> handler)
    {
        return [&](auto L) {
            auto engine = SharedLuaEngine(L);

            handler(engine);

            return 0;
        };
    }

    static std::function<int(lua_State*)> Void_Lua_Function_With_Args(std::string signature, std::function<void(LuaEngine&, LuaArguments&)> handler)
    {
        return [&](auto L) {
            auto engine = SharedLuaEngine(L);
            auto args = LuaArguments(engine, signature);

            handler(engine, args);

            return 0;
        };
    }

    static std::function<int(lua_State*)> Lua_Function(std::function<int(LuaEngine&)> handler)
    {
        return [&](auto L) {
            auto engine = SharedLuaEngine(L);

            return handler(engine);
        };
    }

    static std::function<int(lua_State*)> Lua_Function_With_Args(std::string signature, std::function<int(LuaEngine&, LuaArguments&)> handler)
    {
        return [&](auto L) {
            auto engine = SharedLuaEngine(L);
            auto args = LuaArguments(engine, signature);

            return handler(engine, args);
        };
    }

    /**
     * Follow the directory mapping Lua does: x/y/z becomes x.y.z
     */
    virtual const std::string& Get_Parent_Lua_Module_Path() const
    {
        static const auto module = Nco_Directory.string();

        return module;
    }

    /**
     * Sub-classes should copy/paste this with 'override'
     * to ensure source file metadata is correct.
     */
    virtual const char* Get_Cpp_Source() const
    {
        return __FILE__;
    }
};
