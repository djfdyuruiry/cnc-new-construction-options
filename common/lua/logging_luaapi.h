#pragma once

#include <vector>

#include <spdlog/spdlog.h>

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
                    ).data()
                );

                return 1;
            }).addCFunction("setLevel", [](auto L) {
                auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "Logger.setLevel(<string: level>)");

                arguments.Count_Is(1)
                    .First_Argument_Is<std::string>()
                    .Assert();

                auto level = arguments.Read_First<std::string>().Unpack();

                Assert_Level_Value(engine, level);

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

                auto location = arguments.Read_First<std::string>().Unpack();
                auto level = arguments.Read_Next<std::string>().Unpack();
                auto message = arguments.Read_Next<std::string>().Unpack();

                arguments.Assert_String_Parameter_Is_Valid("location", location);
                Assert_Level_Value(engine, level);

                 auto log_level = spdlog::level::from_str(level.c_str());

                LuaLogger()->log(
                    log_level,
                    std::format("[{}] {}", location, message)
                );

                return 0;
            });
        });
    }

    virtual void Register(LuaEngine& engine) const override
    {
        // reset the logger level, it might have been changed by a Lua setLevel(..) call
        LuaLogger()->set_level(
            DefaultLogLevel
        );

        LuaApi::Register(engine);
    }

protected:
    static inline const auto& LuaLogger = CncLogger::With_Name("Lua");
    static inline spdlog::level::level_enum DefaultLogLevel = LuaLogger()->level();

    static void Assert_Level_Value(const LuaEngine& engine, const std::string& level)
    {
        static const std::vector<spdlog::string_view_t> valid_log_levels SPDLOG_LEVEL_NAMES;

        if (std::find(valid_log_levels.begin(), valid_log_levels.end(), level) == valid_log_levels.end())
        {
            engine.Raise_Error_Format(
                "Invalid level value provided: {}",
                level
            );
        }
    }

    virtual const char* Get_Cpp_Source() const override
    {
        return __FILE__;
    }

};
