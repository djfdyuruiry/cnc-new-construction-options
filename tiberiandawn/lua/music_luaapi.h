#pragma once

#include "externs.h"
#include "td_luaapi.h"
#include "typeconverter.h"

class MusicLuaApi : public TiberianDawnLuaApi
{
public:
    MusicLuaApi() : TiberianDawnLuaApi("Music") {}

    void Register_Functions(LuaEngine& engine) const override;

protected:
    constexpr const char* Get_Cpp_Source() const override
    {
        return __FILE__;
    }
};
