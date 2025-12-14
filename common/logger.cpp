#include <filesystem>

#include "logger.h"
#include "paths.h"

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

void CncLogger::Fatal(const std::string_view message) const
{
    spdlog::get(Name)->critical(message);
    exit(1);
}

std::shared_ptr<spdlog::logger> CncLogger::operator()() const
{
    return spdlog::get(Name);
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

std::shared_ptr<spdlog::async_logger> CncLogger::Build_Logger(const std::string name)
{
    auto logger = std::make_shared<spdlog::async_logger>(
        name,
        Sinks.begin(),
        Sinks.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block
    );

    return logger;
}

void CncLogger::Init_SpdLog()
{
    if (!Load_Env_Log_Levels()) {
        // set a global default if no env config found
        spdlog::set_level(spdlog::level::err);
    }

    spdlog::init_thread_pool(8192, 1);

    Sinks = std::vector<spdlog::sink_ptr>();

    // console logging
    StdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    StdoutSink->set_pattern("%^%L [%!]%$ %v");

    Sinks.emplace_back(StdoutSink);

#ifndef __APPLE__
    // file logging - written to user path
    const auto log_file = std::filesystem::path(Paths.Program_Path()).append(
        std::format("{}.log", DefaultLoggerName)
    );

    RotatingSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        log_file.string(),
        1024 * 1024 * 10,
        3
    );
    RotatingSink->set_pattern( // BUG: Newlines in message strings break JSONL format
        R"({ "time": "%Y-%m-%dT%H:%M:%S.%f%z", "name": "%n", "level": "%^%l%$", "at": "%@", "in": "%!", "process": %P, "thread": %t, "message": "%v" })"
    );

    Sinks.emplace_back(RotatingSink);
#else
    // TODO: macos log path resolve (Library path)
#endif

    // default logger for Debug_String_Log legacy support
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
