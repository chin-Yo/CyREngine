#ifndef LOGGER_H
#define LOGGER_H

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

enum class LogLevel
{
    TRACE = spdlog::level::trace,
    DEBUG = spdlog::level::debug,
    INFO = spdlog::level::info,
    WARN = spdlog::level::warn,
    _ERROR = spdlog::level::err,
    CRITICAL = spdlog::level::critical,
    OFF = spdlog::level::off
};

class Logger
{
public:
    static void Init(const std::string &name = "Logger",
                     LogLevel level = LogLevel::INFO,
                     const std::string &filepath = "",
                     bool async = false,
                     size_t max_file_size = 1024 * 1024 * 5,
                     size_t max_files = 3);

    static std::shared_ptr<spdlog::logger> GetLogger() { return logger_; }

    static void SetLevel(LogLevel level);

    static void Flush()
    {
        if (logger_)
            logger_->flush();
    }

private:
    static std::shared_ptr<spdlog::logger> logger_;
};

#define LOG_TRACE(...) SPDLOG_LOGGER_TRACE(Logger::GetLogger(), __VA_ARGS__);
#define LOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(Logger::GetLogger(), __VA_ARGS__);
#define LOG_INFO(...) SPDLOG_LOGGER_INFO(Logger::GetLogger(), __VA_ARGS__);
#define LOG_WARN(...) SPDLOG_LOGGER_WARN(Logger::GetLogger(), __VA_ARGS__);
#define LOG_ERROR(...) SPDLOG_LOGGER_ERROR(Logger::GetLogger(), __VA_ARGS__);
#define LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(Logger::GetLogger(), __VA_ARGS__);

#define LOG_TRACE_P(...) SPDLOG_LOGGER_TRACE(Logger::GetLogger(), "[{}:{}] {}", __FILE__, __LINE__, fmt::format(__VA_ARGS__));
#define LOG_DEBUG_P(...) SPDLOG_LOGGER_DEBUG(Logger::GetLogger(), "[{}:{}] {}", __FILE__, __LINE__, fmt::format(__VA_ARGS__));
#define LOG_INFO_P(...) SPDLOG_LOGGER_INFO(Logger::GetLogger(), "[{}:{}] {}", __FILE__, __LINE__, fmt::format(__VA_ARGS__));
#define LOG_WARN_P(...) SPDLOG_LOGGER_WARN(Logger::GetLogger(), "[{}:{}] {}", __FILE__, __LINE__, fmt::format(__VA_ARGS__));
#define LOG_ERROR_P(...) SPDLOG_LOGGER_ERROR(Logger::GetLogger(), "[{}:{}] {}", __FILE__, __LINE__, fmt::format(__VA_ARGS__));
#define LOG_CRITICAL_P(...) SPDLOG_LOGGER_CRITICAL(Logger::GetLogger(), "[{}:{}] {}", __FILE__, __LINE__, fmt::format(__VA_ARGS__));

#endif // LOGGER_H