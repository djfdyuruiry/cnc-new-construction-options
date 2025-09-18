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
    SystemLuaApi() : LuaApi("System", true){}

    virtual void Register_Consts(LuaEngine& engine) const override
    {
        With_Api_Namespace(engine, [](auto& n) { 
            n.addConstant("gamePath", Paths.Program_Path())
             .addConstant("luaPath", LuaEngine::Get_Lua_Path().string())
             .addConstant("userPath", Paths.User_Path())
             .addConstant("pathSeparator", Paths.SEP)
             .addConstant("isWindows", IsWindows);
        });
    }

protected:
    virtual const char* Get_Cpp_Source() const override
    {
        return __FILE__;
    }

private:
#ifdef _WIN32
    static inline bool IsWindows = true;
#else
    static inline const bool IsWindows = false;
#endif
};
