#pragma once

#include "luaapi.h"
#include "luaarguments.h"

/**
 * Lua interface for OS operations and path info.
 */
class SystemLuaApi : public LuaApi
{
public:
    SystemLuaApi() : LuaApi("System", true){}

    void Register_Consts(LuaEngine& engine) const override;

protected:
    constexpr const char* Get_Cpp_Source() const override
    {
        return __FILE__;
    }

private:
#ifdef _WIN32
    static constexpr bool IsWindows = true;
#else
    static constexpr bool IsWindows = false;
#endif
};
