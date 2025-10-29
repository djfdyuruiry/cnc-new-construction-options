#include <vector>

#include "logging_luaapi.h"
#include "luaarguments.h"
#include "system_luaapi.h"

// Method implementations
void LoggingLuaApi::Register_Dependencies(LuaEngine& engine) const
{
    engine.Register_Api<SystemLuaApi>();
}

void LoggingLuaApi::Register_Functions(LuaEngine& engine) const
{
    With_Api_Namespace(engine, [](auto& n) {
        n.addCFunction("getLevel", [](auto L) {
            const auto engine = SharedLuaEngine(L);

            engine.Push_Value(
                spdlog::level::to_string_view(
                    LuaLogger()->level()
                ).data()
            );

            return 1;
        }).addCFunction("setLevel", [](auto L) {
            const auto engine = SharedLuaEngine(L);
            auto arguments = LuaArguments(engine, "Logger.setLevel(<string: level>)");

            arguments.Count_Is(1)
                .First_Argument_Is<std::string>()
                .Assert();

            const auto level = arguments.Read_First<std::string>().Unpack();

            Assert_Level_Value(engine, level);

            const auto log_level = spdlog::level::from_str(level);

            LuaLogger()->set_level(log_level);

            return 0;
        }).addCFunction("log", [](auto L) {
            const auto engine = SharedLuaEngine(L);
            auto arguments = LuaArguments(engine, "Logger.log(<string: sourceLocation>, <string: level>, <string: message>)");

            arguments.Count_Is(3)
                .First_Argument_Is<std::string>()
                .Next_Argument_Is<std::string>()
                .Next_Argument_Is<std::string>()
                .Assert();

            const auto location = arguments.Read_First<std::string>().Unpack();
            const auto level = arguments.Read_Next<std::string>().Unpack();
            const auto message = arguments.Read_Next<std::string>().Unpack();

            arguments.Assert_String_Parameter_Is_Valid("location", location);
            Assert_Level_Value(engine, level);

             const auto log_level = spdlog::level::from_str(level);

            LuaLogger()->log(
                log_level,
                std::format("[{}] {}", location, message)
            );

            return 0;
        });
    });
}

void LoggingLuaApi::Register(LuaEngine& engine) const
{
    // reset the logger level, it might have been changed by a Lua setLevel(..) call
    LuaLogger()->set_level(
        DefaultLogLevel
    );

    LuaApi::Register(engine);
}

void LoggingLuaApi::Assert_Level_Value(const LuaEngine& engine, const std::string& level)
{
    static const std::vector valid_log_levels SPDLOG_LEVEL_NAMES;

    if (std::ranges::find(valid_log_levels, level) == valid_log_levels.end()) {
        engine.Raise_Error_Format(
            "Invalid level value provided: {}",
            level
        );
    }
}
