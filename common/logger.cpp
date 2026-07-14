#include "logger.h"

#include <filesystem>

#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include "paths.h"

/**
 * Wrapper around @class{spdlog::sinks::rotating_file_sink_mt}
 *
 * Sanitizes log messages before passing them down to the concrete sink, ensuring characters
 * are escaped and string is safe to use in a JSON string.
 */
class JsonSafeRotatingFileSink final : public spdlog::sinks::sink
{
public:
    JsonSafeRotatingFileSink(spdlog::filename_t base_filename, const std::size_t max_size, const std::size_t max_files)
    {
        FileSink = std::make_unique<spdlog::sinks::rotating_file_sink_mt>(
            std::move(base_filename),
            max_size,
            max_files
        );
    }

    void log(const spdlog::details::log_msg& msg) override
    {
        std::string sanitized_payload;

        for (const auto& c : msg.payload) {
            if (c == '\n') {
                sanitized_payload += "\\n";
            } else if (c == '"') {
                sanitized_payload += "\\\"";
            } else {
                sanitized_payload += c;
            }
        }

        const auto sanitized_message = spdlog::details::log_msg(
            msg.time,
            msg.source,
            msg.logger_name,
            msg.level,
            sanitized_payload
        );

        FileSink->log(sanitized_message);
    }

    void flush() override
    {
        FileSink->flush();
    }

    void set_pattern(const std::string& pattern) override
    {
        FileSink->set_pattern(pattern);
    }

    void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override
    {
        FileSink->set_formatter(std::move(sink_formatter));
    }

private:
    std::unique_ptr<spdlog::sinks::rotating_file_sink_mt> FileSink;
};

const CncLogger& CncLogger::Default()
{
    const static auto default_logger = CncLogger(DefaultLoggerName);

    return default_logger;
}

CncLogger CncLogger::With_Name(const std::string& name)
{
    return {name};
}

void CncLogger::On_Fatal_Error(std::string errorMessage)
{
    OnFatalError(std::move(errorMessage));
}

std::shared_ptr<spdlog::logger> CncLogger::operator()() const
{
    auto logger = spdlog::get(Name);

    return logger;
}

bool CncLogger::Load_Env_Log_Levels()
{
    static std::once_flag onceFlag;
    static auto log_env_defined = false;

    std::call_once(onceFlag, []() {
        log_env_defined = std::getenv("NCO_LOG_LEVEL") != nullptr;
    });

    if (log_env_defined) {
        spdlog::cfg::load_env_levels("NCO_LOG_LEVEL");
    }

    return log_env_defined;
}

std::shared_ptr<spdlog::async_logger> CncLogger::Build_Logger(const std::string& name)
{
    auto logger = std::make_shared<spdlog::async_logger>(
        name,
        Sinks.begin(),
        Sinks.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block
    );

#ifndef REMASTER_BUILD
    // ensure any error/critical messages trigger a flush, due to increased likelihood that the process might crash
    logger->flush_on(spdlog::level::err);
#else
    // BUG: not all log messages reach the file in remaster, so make everything other than debug/trace trigger a flush
    logger->flush_on(spdlog::level::info);
#endif
    return logger;
}

void CncLogger::Init_SpdLog()
{
    if (!Load_Env_Log_Levels()) {
        // set a global default if no env config found
        spdlog::set_level(spdlog::level::err);
    }

    Sinks.clear();

    spdlog::init_thread_pool(8192, 1);

// remaster dll has no stdout handle, so disable console logging in that build
#ifndef REMASTER_BUILD
    // console logging
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    stdout_sink->set_pattern("%^%L [%=15!n] %v%$");

    Sinks.emplace_back(std::move(stdout_sink));
#endif

    // create log file in user path, filename matches program binary (nco-td.log, TIBERIANDAWN.DLL.log etc.)
    const auto log_file_name = std::format("{}.log", PathsClass::Try_Get_Program_Binary_Name());

    const auto log_file = std::filesystem::path(PathsClass::Try_Get_User_Path_Root())
        .append(log_file_name)
        .string();

    auto rotating_sink = std::make_shared<JsonSafeRotatingFileSink>(
        log_file,
        1024 * 1024 * 10,
        3
    );
    rotating_sink->set_pattern(
        R"({ "time": "%T.%e", "level": "%L", "message": "%v", "logger": "%n", "at": "%s:%#", "in": "%!", "date": "%d-%m-%C", "process": %P, "thread": %t })"
    );

    Sinks.emplace_back(std::move(rotating_sink));

    // default logger for Debug_String_Log legacy support, see debugstring.cpp
    spdlog::set_default_logger(
        Build_Logger(DefaultLoggerName)
    );
}

void CncLogger::Register(const std::string& name)
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

CncLogger::CncLogger(const std::string& name): Name(name)
{
    Register(name);
}
