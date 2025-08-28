#include <assert.h>
#include <format>
#include <memory>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <spdlog/spdlog.h>

#include "logger.h"

/**
 * Main log function, intended to be used from behind macros that pass in file and line details.
 */
void Debug_String_Log(unsigned level, const char* file, int line, const char* function_or_method, const char* fmt, ...)
{
    static const char* levels[] = {"off", "critical", "error", "warn", "info", "debug", "trace"};
    assert(level <= 6);

    auto spd_level = spdlog::level::from_str(levels[level]);

    if (!CncLogger::Default()->should_log(spd_level)) {
        return;
    }

    va_list args;

    // get message_size
    va_start(args, fmt);
    auto message_size = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (message_size < 0) {
        CncLogger::Default()->error(
            "vsnprintf failed to process legacy log message in Debug_String_Log. source={}:{} | fmt={}",
            file,
            line,
            fmt
        );
        return;
    }

    // format the message
    std::unique_ptr<char[]> formatted_message(new char[message_size + 1]);

    va_start(args, fmt);
    auto result = vsnprintf(formatted_message.get(), message_size + 1, fmt, args);
    va_end(args);

    if (result < 0) {
        CncLogger::Default()->error(
            "vsnprintf failed to process legacy log message in Debug_String_Log. source={}:{} | fmt={}",
            file,
            line,
            fmt
        );
        return;
    }

    // log message using CncLogger
    auto message = std::string(formatted_message.get());

    if (spd_level == spdlog::level::critical) {
        CncLogger::Default.Fatal(message);
    }

    CncLogger::Default()->log(
        spdlog::source_loc{file, line, function_or_method},
        spd_level,
        message
    );
}
