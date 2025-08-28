#include <spdlog/async.h>
#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include "logger.h"

// static variables
static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> stdout_sink;
static std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> rotating_sink;
static std::vector<spdlog::sink_ptr> sinks;

// static functions
/**
 * Load spdlog level setting from the environment.
 */
static bool Load_Env_Log_Levels() {
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
 
// class members
const std::string CncLogger::DefaultLoggerName = std::string("nco");
const CncLogger CncLogger::Default = CncLogger(CncLogger::DefaultLoggerName);

// class methods
void CncLogger::Register(const std::string name)
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

CncLogger::CncLogger(const std::string name) : Name(name)
{
    Register(name);
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
