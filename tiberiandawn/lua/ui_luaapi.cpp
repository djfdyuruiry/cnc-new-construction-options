#include "../common/lua/luaarguments.h"

#include "../function.h"

#include "events/popup_luaevent.h"
#include "ui_luaapi.h"

void UiLuaApi::Register_Functions(LuaEngine& engine) const
{
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