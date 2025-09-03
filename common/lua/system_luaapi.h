#pragma once

#include "../paths.h"

#include "luaapi.h"
#include "luaarguments.h"

/**
 * Lua interface for OS operations and path info.
 */
class SystemLuaApi : public LuaApi
{
public:
    SystemLuaApi() : LuaApi("System", { "System.lua" }){}

    virtual void Register_Consts(LuaEngine& engine) const override {
        With_Api_Namespace(engine, [](auto& n) { 
            n.addConstant("gamePath", Program_Path)
             .addConstant("luaPath", LuaEngine::Lua_Path.string())
             .addConstant("userPath", User_Path)
             .addConstant("pathSeparator", Paths.SEP)
             .addConstant("isWindows", Is_Windows);
        });
    }

private:
#ifdef _WIN32
    static inline bool Is_Windows = true;
#else
    static inline const bool Is_Windows = false;
#endif
    static inline const std::string User_Path = std::string(Paths.User_Path());
    static inline const std::string Program_Path = std::string(Paths.Program_Path());
};
