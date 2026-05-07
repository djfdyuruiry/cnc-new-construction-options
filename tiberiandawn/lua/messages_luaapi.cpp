#include "common/lua/luaarguments.h"

#include "function.h"

#include "events/addmessage_luaevent.h"
#include "messages_luaapi.h"

void MessagesLuaApi::Register_Functions(LuaEngine& engine) const
{
    With_Api_Namespace(engine, [](auto& n) {
        n.addCFunction("sendToPlayer", [](auto L) {
            const auto engine = SharedLuaEngine(L);
            auto arguments = LuaArguments(engine, "Messages.sendToPlayer(<string: message>[, <string: colour>])");

            arguments.Count_Is(2)
                .First_Argument_Is<std::string>()
                .Next_Argument_Is<std::string>()
                .Assert();

            const auto message = arguments.Read_First<std::string>().Unpack();
            const auto colour_str = arguments.Read_Next<std::string>().Unpack();

            const auto colour = TdTypeConverter::Try_Parse<ColorType>(colour_str);

            if (!colour.has_value()) {
                engine.Raise_Error_Format(
                    "Failed to parse instance string '{}' as type: {} | valid_values={}",
                    colour_str,
                    TdTypeConverter::Get_Type_Name<ColorType>(),
                    CncStringUtils::To_Csv(TdTypeConverter::Get_Valid_Strings<ColorType>())
                );
            }

            LuaList.Push<AddMessageLuaEvent>(message, *colour);

            return 0;
        });
    });
}
