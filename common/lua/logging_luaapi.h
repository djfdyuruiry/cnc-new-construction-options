#pragma once

#include "logger.h"
#include "luaapi.h"

/**
 * Lua interface for spdlog.
 */
class LoggingLuaApi : public LuaApi
{
public:
    LoggingLuaApi() : LuaApi("Logger", true) {}

    void Register_Dependencies(LuaEngine& engine) const override;
    void Register_Functions(LuaEngine& engine) const override;
    void Register(LuaEngine& engine) const override;

protected:
    static inline const auto& LuaLogger = CncLogger::With_Name("Lua");
    static inline spdlog::level::level_enum DefaultLogLevel = LuaLogger()->level();

    static void Assert_Level_Value(const LuaEngine& engine, const std::string& level);

    constexpr const char* Get_Cpp_Source() const override
    {
        return __FILE__;
    }
};
