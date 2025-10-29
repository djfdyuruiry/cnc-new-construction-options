#include "../../common/lua/luaarguments.h"

#include "../function.h"

#include "events/addmessage_luaevent.h"
#include "messages_luaapi.h"

void MessagesLuaApi::Register_Functions(LuaEngine& engine) const {
    With_Api_Namespace(engine, [](auto& n) {
        n.addCFunction("sendToPlayer", [](auto L) {
            const auto engine = SharedLuaEngine(L);
            auto arguments = LuaArguments(engine, "Messages.sendToPlayer(<string: message>)");

            arguments.Count_Is(1)
                .First_Argument_Is<std::string>()
                .Assert();

            const auto message = arguments.Read_First<std::string>().Unpack();

            LuaList.Push<AddMessageLuaEvent>(message);

            return 0;
        });
    });
}
