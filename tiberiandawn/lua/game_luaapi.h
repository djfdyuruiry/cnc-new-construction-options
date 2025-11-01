#pragma once

#include "td_luaapi.h"

class GameLuaApi: public TiberianDawnLuaApi
{
public:
    GameLuaApi() : TiberianDawnLuaApi("Game", true) {}

    void Register_Functions(LuaEngine& engine) const override;

protected:
    constexpr const char* Get_Cpp_Source() const override
    {
        return __FILE__;
    }
};
