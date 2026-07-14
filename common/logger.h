#pragma once

#include <memory>

// set SPDLOG_FUNCTION to platform specific macro for more detailed function signatures
#ifndef SPDLOG_FUNCTION
    #if defined(__GNUC__) || defined(__clang__)
        #define SPDLOG_FUNCTION static_cast<const char*>(__PRETTY_FUNCTION__)
    #elif defined(_MSC_VER)
        #define SPDLOG_FUNCTION static_cast<const char*>(__FUNCSIG__)
    #else
        // fallback, supported by all C++ compilers
        #define SPDLOG_FUNCTION static_cast<const char*>(__FUNCTION__)
    #endif
#endif

#include <spdlog/async.h>
#include <spdlog/spdlog.h>

#ifndef TRIGGER_DEBUGGER
    #ifdef _DEBUG
        // reference platform specific macro/function
        #ifdef _WIN32
            #include <intrin.h>
            #define TRIGGER_DEBUGGER __debugbreak()
        #elif defined(__GNUC__) && !defined(__clang__)
            #define TRIGGER_DEBUGGER __builtin_trap()
        #elif defined(__GNUC__) && defined(__clang__)
            #define TRIGGER_DEBUGGER __builtin_debugtrap()
        #else
            // fallback, use C standard error handling
            #define TRIGGER_DEBUGGER #include <assert.h>; \
                assert(false)
        #endif
    #else
        // no-op if not building Debug version
        #define TRIGGER_DEBUGGER (void)0
    #endif
#endif

/**
 * @brief Logger class using spdlog. For memory and thread safety, only assign instances
 *        of this class using static storage specifiers; use a static member or variable.
 */
class CncLogger
{
public:
    static inline const auto DefaultLoggerName = std::string("nco");
    static inline std::function<void(std::string)> OnFatalError = [](const auto& e) {
        spdlog::shutdown();

        TRIGGER_DEBUGGER;

        throw std::runtime_error(e);
    };

    static const CncLogger& Default();

    static CncLogger With_Name(const std::string& name);

    template<class... Args>
    static void On_Fatal_Error(std::format_string<Args...> fmt, Args&&... args)
    {
        OnFatalError(
            std::format(fmt, std::forward<Args>(args)...)
        );
    }

    static void On_Fatal_Error(std::string errorMessage);

    // TODO: Add PII level/log method to require a special flag or runtime arg to force print them (paths containing usernames etc.)

    std::shared_ptr<spdlog::logger> operator()() const;

private:
    static inline std::vector<spdlog::sink_ptr> Sinks;

    const std::string Name;

    static bool Load_Env_Log_Levels();

    static std::shared_ptr<spdlog::async_logger> Build_Logger(const std::string& name);

    static void Init_SpdLog();

    static void Register(const std::string& name);

    CncLogger(const std::string& name);
};

// 'virtual' static method to build a logger with a symbol as it's name (class/function etc.)
#define For(TYPE_OR_FUNCTION) With_Name(#TYPE_OR_FUNCTION)

// alias SPD macros so we don't pollute code with SPD refs
#define CNC_LOG(...) CncLogger::Default()()->log(__VA_ARGS__)

#define CNC_LOG_TRACE(...) SPDLOG_LOGGER_CALL(CncLogger::Default()(), spdlog::level::trace, __VA_ARGS__)

#define CNC_LOG_DEBUG(...) SPDLOG_LOGGER_CALL(CncLogger::Default()(), spdlog::level::debug, __VA_ARGS__)

#define CNC_LOG_INFO(...) SPDLOG_LOGGER_CALL(CncLogger::Default()(), spdlog::level::info, __VA_ARGS__)

#define CNC_LOG_WARN(...) SPDLOG_LOGGER_CALL(CncLogger::Default()(), spdlog::level::warn, __VA_ARGS__)

#define CNC_LOG_ERROR(...) SPDLOG_LOGGER_CALL(CncLogger::Default()(), spdlog::level::err, __VA_ARGS__)

#define CNC_LOG_CRITICAL(...) SPDLOG_LOGGER_CALL(CncLogger::Default()(), spdlog::level::critical, __VA_ARGS__)

#define CNC_LOG_FATAL(...) \
    SPDLOG_LOGGER_CALL(CncLogger::Default()(), spdlog::level::critical, __VA_ARGS__); \
    CncLogger::On_Fatal_Error(__VA_ARGS__)

#define CNC_LOGGER_TRACE(...) SPDLOG_LOGGER_CALL(Logger(), spdlog::level::trace, __VA_ARGS__)

#define CNC_LOGGER_DEBUG(...) SPDLOG_LOGGER_CALL(Logger(), spdlog::level::debug, __VA_ARGS__)

#define CNC_LOGGER_INFO(...) SPDLOG_LOGGER_CALL(Logger(), spdlog::level::info, __VA_ARGS__)

#define CNC_LOGGER_WARN(...) SPDLOG_LOGGER_CALL(Logger(), spdlog::level::warn, __VA_ARGS__)

#define CNC_LOGGER_ERROR(...) SPDLOG_LOGGER_CALL(Logger(), spdlog::level::err, __VA_ARGS__)

#define CNC_LOGGER_CRITICAL(...) SPDLOG_LOGGER_CALL(Logger(), spdlog::level::critical, __VA_ARGS__)

#define CNC_LOGGER_FATAL(...) \
    SPDLOG_LOGGER_CALL(Logger(), spdlog::level::critical, __VA_ARGS__); \
    CncLogger::On_Fatal_Error(__VA_ARGS__)
