#include <algorithm>
#include <assert.h>
#include <format>
#include <memory>
#include <ranges>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <spdlog/spdlog.h>

#include "logger.h"

/**
 * Main log function, intended to be used from behind macros that pass in file and line details.
 */
void Debug_String_Log(unsigned level, const char* file, int line, const char* function_or_method, const char* fmt, ...)
{
    assert(level <= 6);

    static std::vector levels SPDLOG_LEVEL_NAMES;
    static std::once_flag levels_init;

    std::call_once(levels_init, []() {
        // reverse to match level param logic ( level indicates (0)off->trace(6), but SPDLOG_LEVEL_NAMES is (0)trace->off(6))
        std::ranges::reverse(levels);
    });

    auto spd_level = spdlog::level::from_str(
        levels.at(level).data()
    );

    if (!CncLogger::Default()()->should_log(spd_level)) {
        return;
    }

    va_list args;

    // get message_size
    va_start(args, fmt);
    const auto message_size = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (message_size < 0) {
        CNC_LOG_ERROR(
            "vsnprintf failed to process legacy log message in Debug_String_Log. source={}:{} | fmt={}",
            file,
            line,
            fmt
        );
        return;
    }

    // format the message
    const std::unique_ptr<char[]> formatted_message(new char[message_size + 1]);

    va_start(args, fmt);
    const auto result = vsnprintf(formatted_message.get(), message_size + 1, fmt, args);
    va_end(args);

    if (result < 0) {
        CNC_LOG_ERROR(
            "vsnprintf failed to process legacy log message in Debug_String_Log. source={}:{} | fmt={}",
            file,
            line,
            fmt
        );
        return;
    }

    // log message using CncLogger
    const auto message = std::string(formatted_message.get());

    if (spd_level == spdlog::level::critical) {
        CNC_LOG_FATAL(message);
    }

    CNC_LOG(
        spdlog::source_loc{file, line, function_or_method},
        spd_level,
        message
    );
}
