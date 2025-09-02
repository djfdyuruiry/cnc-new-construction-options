#pragma once

#include "lua/luaapi.h"

class LoggingLuaApi : public LuaApi
{
public:
    LoggingLuaApi(const LuaEngine& engine)
        : LuaApi(engine, "Logger")
    {
    }

    virtual void Register_Functions() const override
    {
        Get_Namespace()
            .addCFunction("log", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Logger.log(<string: level>, <string: message>)");

                if (!arguments.Count_Is(3)
                        .First_Argument_Is<std::string>()
                        .Next_Argument_Is<std::string>()
                        .Next_Argument_Is<std::string>()
                        .Assert()) {
                    return 0;
                }

                auto location = arguments.Read_First<std::string>().Unpack();
                auto level = arguments.Read_Next<std::string>().Unpack();
                auto message = arguments.Read_Next<std::string>().Unpack();

                auto log_level = spdlog::level::from_str(level.data());

                Logger()->log(
                    log_level,
                    std::format("[{}] {}", location, message)
                );
 
                return 0;
            })
        .endNamespace();
    }
};
