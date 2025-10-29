#pragma once

#include <memory>

#include "../common/lua/luaapi.h"
#include "../common/lua/luaarguments.h"

#include "events/addmessage_luaevent.h"
#include "td_luaapi.h"

class MessagesLuaApi: public TiberianDawnLuaApi
{
public:
    MessagesLuaApi() : TiberianDawnLuaApi("Messages", true) {}

    virtual void Register_Functions(LuaEngine& engine) const override
    {
        With_Api_Namespace(engine, [](auto& n) {
            n.addCFunction("sendToPlayer", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Messages.sendToPlayer(<string: message>)");

                arguments.Count_Is(1)
                    .First_Argument_Is<std::string>()
                    .Assert();

                auto message = arguments.Read_First<std::string>().Unpack();

                LuaList.Push<AddMessageLuaEvent>(message);

                return 0;
            });
        });
    }

protected:
    constexpr const char* Get_Cpp_Source() const override
    {
        return __FILE__;
    }
};
