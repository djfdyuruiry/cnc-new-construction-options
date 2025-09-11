#pragma once

#include "luaapi.h"
#include "luaarguments.h"
#include "system_luaapi.h"

/**
 * Lua interface for spdlog.
 */
class LoggingLuaApi : public LuaApi
{
public:
    LoggingLuaApi() : LuaApi("Logger", true) {}

    virtual void Register_Dependencies(LuaEngine& engine) const override
    {
        engine.Register_Api<SystemLuaApi>();
    }

    virtual void Register_Functions(LuaEngine& engine) const override
    {
        With_Api_Namespace(engine, [](auto& n) {
            n.addCFunction("getLevel", [](auto L) {
                auto engine = SharedLuaEngine(L);

                engine.Push_Value(
                    spdlog::level::to_string_view(
                        LuaLogger()->level()
                    )
                );

                return 1;
            }).addCFunction("setLevel", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Logger.setLevel(<string: level>)");

                arguments.Count_Is(1)
                    .First_Argument_Is<std::string>()
                    .Assert();

                // TODO: validation
                auto level = arguments.Read_First<std::string>().Unpack();

                auto log_level = spdlog::level::from_str(level.c_str());

                LuaLogger()->set_level(log_level);

                return 0;
            })
            .addCFunction("log", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Logger.log(<string: sourceLocation>, <string: level>, <string: message>)");

                arguments.Count_Is(3)
                    .First_Argument_Is<std::string>()
                    .Next_Argument_Is<std::string>()
                    .Next_Argument_Is<std::string>()
                    .Assert();

                // TODO: validation
                auto location = arguments.Read_First<std::string>().Unpack();
                auto level = arguments.Read_Next<std::string>().Unpack();
                auto message = arguments.Read_Next<std::string>().Unpack();

                auto log_level = spdlog::level::from_str(level.c_str());

                LuaLogger()->log(
                    log_level,
                    std::format("[{}] {}", location, message)
                );

                return 0;
            });
        });
    }

protected:
    inline static const CncLogger LuaLogger = CncLogger("Lua");

    virtual const char* Get_Cpp_Source() const override
    {
        return __FILE__;
    }

};
