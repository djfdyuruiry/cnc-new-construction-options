#pragma once


#include "td_luaapi.h"

class MessagesLuaApi: public TiberianDawnLuaApi
{
public:
    MessagesLuaApi() : TiberianDawnLuaApi("Messages", true) {}

    void Register_Functions(LuaEngine& engine) const override;

protected:
    constexpr const char* Get_Cpp_Source() const override
    {
        return __FILE__;
    }
};
