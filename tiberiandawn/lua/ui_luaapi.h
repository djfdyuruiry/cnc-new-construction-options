#pragma once

#include <memory>

#include "../common/lua/luaarguments.h"
#include "../common/lua/luaapi.h"

#include "td_luaevents.h"
#include "../externs.h"

#include "td_luaapi.h"
#include "td_luaevents.h"

class UiLuaApi: public TiberianDawnLuaApi
{
public:
    UiLuaApi() : TiberianDawnLuaApi("UI", { "UI.lua" }) {}

    virtual void Register_Functions(LuaEngine& engine) const override {
        With_Api_Namespace(engine, [](auto& n) {
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

protected:
    virtual const char* Get_Cpp_Source() const override {
        return __FILE__;
    }

};
