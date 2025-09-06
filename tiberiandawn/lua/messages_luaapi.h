#pragma once

#include <memory>

#include "../common/lua/luaapi.h"
#include "../common/lua/luaarguments.h"

#include "../externs.h"

#include "td_luaapi.h"

class MessagesLuaApi: public TiberianDawnLuaApi
{
public:
    MessagesLuaApi() : TiberianDawnLuaApi("Messages", true) {}

    virtual void Register_Functions(LuaEngine& engine) const override {
        With_Api_Namespace(engine, [](auto& n) {
            n.addCFunction("showToPlayer", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Messages.showToPlayer(<string: message>)");

                arguments.Count_Is(1)
                    .First_Argument_Is<std::string>()
                    .Assert();

                auto message = arguments.Read_First<std::string>().Unpack();

                // ripped off from netdlg.cpp
                Messages.Add_Message(
                    message.data(),
                    CC_GREEN,
                    TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW,
                    600,
                    0,
                    0
                );
                Map.Flag_To_Redraw(false);

                return 0;
            })
        });
    }

protected:
    virtual const char* Get_Cpp_Source() const override {
        return __FILE__;
    }

};
