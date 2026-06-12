#pragma once

#include <filesystem>
#include <functional>
#include <string>

#include "logger.h"

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
    static inline const std::filesystem::path Nco_Directory = "nco";
    static constexpr std::string_view Require_Global = "require";

    const std::string_view Name;
    const bool Has_Native_Module;

    LuaApi(const std::string_view name)
        : Name(name), Has_Native_Module(true) {}

    LuaApi(const std::string_view name, const bool has_native_module)
        : Name(name), Has_Native_Module(has_native_module) {}

    LuaApi(const std::string_view name, const bool has_native_module, std::vector<std::filesystem::path> scripts)
        : Name(name), Has_Native_Module(has_native_module), Scripts(std::move(scripts)) {}

    virtual ~LuaApi() = default;

    void With_Api_Namespace(const LuaEngine& engine, const std::function<void(luabridge::Namespace&)>& action) const;

    void Register_Api_Metadata(const LuaEngine& engine) const;

    /**
     * Ensure the host Lua engine has other APIs
     * that are a dependency of this API.
     */
    virtual void Register_Dependencies(LuaEngine& engine) const;

    virtual void Register_Consts(LuaEngine& engine) const;

    virtual void Register_Functions(LuaEngine& engine) const;

    virtual void Register_Native_Module(LuaEngine& engine) const;

    virtual void Register_Scripts(LuaEngine& engine) const;

    virtual void Register(LuaEngine& engine) const;

protected:
    static inline const auto& Logger = CncLogger::For(LuaApi);

    std::vector<std::filesystem::path> Scripts;

    /**
     * Follow the directory mapping Lua does: x/y/z becomes x.y.z
     */
    virtual const std::string& Get_Parent_Lua_Module_Path() const;

    /**
     * Subclasses should copy/paste this with 'override'
     * to ensure source file metadata is correct.
     */
    virtual constexpr const char* Get_Cpp_Source() const
    {
        return __FILE__;
    }
};
