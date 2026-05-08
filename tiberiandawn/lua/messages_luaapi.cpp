#include "common/lua/luaarguments.h"

#include "function.h"

#include "events/addmessage_luaevent.h"
#include "messages_luaapi.h"

void MessagesLuaApi::Register_Functions(LuaEngine& engine) const
{
    With_Api_Namespace(engine, [](auto& n) {
        n.addCFunction("sendToPlayer", [](auto L) {
            const auto engine = SharedLuaEngine(L);
            auto arguments = LuaArguments(
                engine,
                "Messages.sendToPlayer(<string: message>, <string: colour>, <number: timeoutInSeconds>)"
            );

            arguments.Count_Is(3)
                .First_Argument_Is<std::string>()
                .Next_Argument_Is<std::string>()
                .Next_Argument_Is<int>()
                .Assert();

            const auto message = arguments.Read_First<std::string>().Unpack();

            const auto colour_str = arguments.Read_Next<std::string>().Unpack();
            const auto colour = TdTypeConverter::Assert_Parse_Lua_String<CCPaletteType>(engine, colour_str);

            const auto timeoutInSecs = arguments.Read_Next<int>().Unpack();

            LuaList.Push<AddMessageLuaEvent>(message, colour, timeoutInSecs * TICKS_PER_SECOND);

            return 0;
        });

        n.addCFunction("validateColorType", [](auto L) {
            const auto engine = SharedLuaEngine(L);
            auto arguments = LuaArguments(engine, "Messages.validateColorType(<string: colour>)");

            arguments.Count_Is(1)
                .First_Argument_Is<std::string>()
                .Assert();

            const auto colour_str = arguments.Read_First<std::string>().Unpack();

            TdTypeConverter::Assert_Parse_Lua_String<CCPaletteType>(engine, colour_str);

            return 0;
        });
    });
}
