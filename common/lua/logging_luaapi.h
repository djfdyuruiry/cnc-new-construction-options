#pragma once

#include "luaapi.h"

class LoggingLuaApi : public LuaApi
{
public:
    LoggingLuaApi(const LuaEngine& engine)
        : LuaApi(engine, "Logger")
    {
    }

    virtual void Register_Functions() const
    {
        Get_Namespace()
            .addCFunction("log", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Logger.log(<string: level>, <string: message>)");

                if (!arguments.Count_Is(2)
                        .First_Argument_Is<std::string>()
                        .Next_Argument_Is<std::string>()
                        .Assert()) {
                    return 0;
                }

                auto level = arguments.Read_First<std::string>();
                auto message = arguments.Read_Next<std::string>();

                auto log_level = spdlog::level::from_str(level.Value_Source.value().data());
                
                Logger()->log(log_level, message.Value_Source.value().data());
 
                return 0;
            })
        .endNamespace();
    }
};
