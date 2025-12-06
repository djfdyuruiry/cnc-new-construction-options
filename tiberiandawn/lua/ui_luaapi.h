#pragma once

#include "common/lua/luaengine.h"

#include "td_luaapi.h"

class UiLuaApi: public TiberianDawnLuaApi
{
public:
    UiLuaApi() : TiberianDawnLuaApi("UI", true) {}

     void Register_Functions(LuaEngine& engine) const override
    ;

protected:
    constexpr const char* Get_Cpp_Source() const override
    {
        return __FILE__;
    }
};
