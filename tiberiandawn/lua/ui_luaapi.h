#pragma once

#include <memory>

#include "../common/lua/luaarguments.h"
#include "../common/lua/luaapi.h"

#include "td_luaevents.h"
#include "../externs.h"

class UiLuaApi: public LuaApi {
public:
    UiLuaApi(const LuaEngine& engine) : LuaApi(engine, "UI", { "UI.lua" }) {}

    virtual void Register_Functions() const override {
        With_Api_Namespace([](auto& n) {
            n.addCFunction("popupOk", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "UI.popupOk(<string: message>)");

                arguments.Count_Is(1)
                    .First_Argument_Is<std::string>()
                    .Assert();

                auto message = arguments.Read_First<std::string>().Unpack();

                LuaList.Push<PopupLuaEvent>(message);

                return 0;
            });
        });
    }
};
