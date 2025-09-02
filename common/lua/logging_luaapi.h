#pragma once

#include "luaapi.h"

/**
 * Lua interface for spdlog.
 */
class LoggingLuaApi : public LuaApi
{
public:
    LoggingLuaApi(const LuaEngine& engine)
        : LuaApi(
            engine,
            "Logger",
            { "Logger.lua" }
        ){}

    virtual void Register_Consts() const override {
        With_Api_Namespace([](auto& n) {
            auto log_level = spdlog::level::to_string_view(spdlog::get_level()).data();

            n.addConstant("level", log_level);
        });
    }

    virtual void Register_Functions() const override {
        With_Api_Namespace([](auto& n) {
            n.addCFunction("log", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Logger.log(<string: level>, <string: message>)");

                arguments.Count_Is(3)
                    .First_Argument_Is<std::string>()
                    .Next_Argument_Is<std::string>()
                    .Next_Argument_Is<std::string>()
                    .Assert();

                auto location = arguments.Read_First<std::string>().Unpack();
                auto level = arguments.Read_Next<std::string>().Unpack();
                auto message = arguments.Read_Next<std::string>().Unpack();

                auto log_level = spdlog::level::from_str(level.c_str());

                Logger()->log(
                    log_level,
                    std::format("Lua [{}] {}", location, message)
                );

                return 0;
            });
        });
    }
};
