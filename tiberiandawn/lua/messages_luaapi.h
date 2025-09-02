#pragma once

#include <memory>

#include "../common/lua/luaapi.h"

#include "td_luaevents.h"
#include "../externs.h"

class MessagesLuaApi: public LuaApi {
public:
    MessagesLuaApi(const LuaEngine& engine) : LuaApi(engine, "Messages", { "Messages.lua" }) {}

    virtual void Register_Functions() const override {
        With_Api_Namespace([](auto& n) {
            n.addCFunction("showToPlayer", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Messages.showToPlayer(<string: message>)");

                if (!arguments.Count_Is(1)
                        .First_Argument_Is<std::string>()
                        .Assert()) {
                    return 0;
                }

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
            .addCFunction("popupOk", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Messages.popupOk(<string: message>)");

                if (!arguments.Count_Is(1)
                        .First_Argument_Is<std::string>()
                        .Assert()) {
                    return 0;
                }

                auto message = arguments.Read_First<std::string>().Unpack();

                LuaList.Push<PopupLuaEvent>(message);

                return 0;
            });
        });
    }
};
