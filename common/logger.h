#pragma once

#include <memory>
#include <string_view>

/**
 * spdlog static configuration
 */

// pull in function call info for current compiler
#if defined(__GNUC__) || defined(__clang__)
    #define SPDLOG_FUNCTION static_cast<const char*>(__PRETTY_FUNCTION__)
#elif defined(_MSC_VER)
    #define SPDLOG_FUNCTION static_cast<const char*>(__FUNCSIG__)
#endif

#include <spdlog/async.h>
#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

/**
 * @brief Logger class using spdlog. For memory and thread safety, only assign instances
 *        of this class using static storage specifiers; use a static member or variable.
 */
class CncLogger
{
public:
    inline static const std::string DefaultLoggerName = std::string("nco");
    
    static const CncLogger& Default() {
        const static auto default_logger = CncLogger(DefaultLoggerName);

        return default_logger;
    }

    CncLogger(const std::string name) : Name(name)
    {
        Register(name);
    }

    // TODO: Add PII level/log method to require a special flag or runtime arg to force print them (paths containing usernames etc.)
    void Fatal(const std::string_view message) const
    {
        spdlog::get(Name)->critical(message);
        exit(1);
    }

    std::shared_ptr<spdlog::logger> operator()() const
    {
        return spdlog::get(Name);
    }

private:
    inline static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> stdout_sink;
    inline static std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> rotating_sink;
    inline static std::vector<spdlog::sink_ptr> sinks;

    static bool Load_Env_Log_Levels()
    {
        static std::once_flag onceFlag;
        static auto log_env_defined = false;

        std::call_once(onceFlag, []() {
            auto log_env_var = std::getenv("NCO_LOG_LEVEL");

            log_env_defined = log_env_var != nullptr;
        });

        if (log_env_defined) {
            spdlog::cfg::load_env_levels("NCO_LOG_LEVEL");
        }

        return log_env_defined;
    }

    static std::shared_ptr<spdlog::async_logger> Build_Logger(const std::string name) {
        auto logger = std::make_shared<spdlog::async_logger>(
            name,
            sinks.begin(),
            sinks.end(),
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block
        );

        return logger;
    }

    static void Init_SpdLog() {
        if (!Load_Env_Log_Levels()) {
            // set a global default if no env config found
            spdlog::set_level(spdlog::level::err);
        }

        spdlog::init_thread_pool(8192, 1);

        auto log_file = std::format("{}.log", CncLogger::DefaultLoggerName);

        stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file, 1024 * 1024 * 10, 3);
        sinks = std::vector<spdlog::sink_ptr>{stdout_sink, rotating_sink};

        stdout_sink.get()->set_pattern("%^%L [%!]%$ %v");
        // BUG: Newlines in message strings break JSONL format
        rotating_sink.get()->set_pattern(
            R"({ "time": "%Y-%m-%dT%H:%M:%S.%f%z", "name": "%n", "level": "%^%l%$", "at": "%@", "in": "%!", "process": %P, "thread": %t, "message": "%v" })"
        );

        spdlog::set_default_logger(
            Build_Logger(CncLogger::DefaultLoggerName)
        );
    }

    static void Register(const std::string name)
    {
        static std::once_flag onceFlag;

        std::call_once(onceFlag, Init_SpdLog);

        if (name == CncLogger::DefaultLoggerName) {
            // default logger registered in Ensure_SpdLog_Initialised (exactly once)
            return;
        }

        spdlog::register_or_replace(
            Build_Logger(name)
        );

        // ensure any env var config is applied to the new logger
        Load_Env_Log_Levels();
    }

    const std::string Name;
};

// alias SPD macros so we don't pollute code with SPD refs
#define CNC_LOG_TRACE(...) SPDLOG_LOGGER_CALL(CncLogger::Default()(), spdlog::level::trace, __VA_ARGS__)

#define CNC_LOG_DEBUG(...) SPDLOG_LOGGER_CALL(CncLogger::Default()(), spdlog::level::debug, __VA_ARGS__)

#define CNC_LOG_INFO(...) SPDLOG_LOGGER_CALL(CncLogger::Default()(), spdlog::level::info, __VA_ARGS__)

#define CNC_LOG_WARN(...) SPDLOG_LOGGER_CALL(CncLogger::Default()(), spdlog::level::warn, __VA_ARGS__)

#define CNC_LOG_ERROR(...) SPDLOG_LOGGER_CALL(CncLogger::Default()(), spdlog::level::error, __VA_ARGS__)

#define CNC_LOG_CRITICAL(...) SPDLOG_LOGGER_CALL(CncLogger::Default()(), spdlog::level::critical, __VA_ARGS__)

// TODO: trigger debugger to break here
#define CNC_LOG_FATAL(...) \
    SPDLOG_LOGGER_CALL(CncLogger::Default()(), spdlog::level::critical, __VA_ARGS__); \
    exit(1)

#define CNC_LOGGER_TRACE(...) SPDLOG_LOGGER_CALL(Logger(), spdlog::level::trace, __VA_ARGS__)

#define CNC_LOGGER_DEBUG(...) SPDLOG_LOGGER_CALL(Logger(), spdlog::level::debug, __VA_ARGS__)

#define CNC_LOGGER_INFO(...) SPDLOG_LOGGER_CALL(Logger(), spdlog::level::info, __VA_ARGS__)

#define CNC_LOGGER_WARN(...) SPDLOG_LOGGER_CALL(Logger(), spdlog::level::warn, __VA_ARGS__)

#define CNC_LOGGER_ERROR(...) SPDLOG_LOGGER_CALL(Logger(), spdlog::level::error, __VA_ARGS__)

#define CNC_LOGGER_CRITICAL(...) SPDLOG_LOGGER_CALL(Logger(), spdlog::level::critical, __VA_ARGS__)

// TODO: trigger debugger to break here
#define CNC_LOGGER_FATAL(...) \
    SPDLOG_LOGGER_CALL(Logger(), spdlog::level::critical, __VA_ARGS__); \
    exit(1)
