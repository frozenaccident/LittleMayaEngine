#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>

#include <memory>

class Logger
{
public:
    static void init();
    static std::shared_ptr<spdlog::async_logger>& getLogger();

private:
    static std::shared_ptr<spdlog::async_logger> logger;
};

#define LOG_TRACE(...)  ::Logger::getLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)  ::Logger::getLogger()->debug(__VA_ARGS__)
#define LOG_INFO(...)   ::Logger::getLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)   ::Logger::getLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)  ::Logger::getLogger()->error(__VA_ARGS__)
#define LOG_FATAL(...)  ::Logger::getLogger()->critical(__VA_ARGS__)
#define LOG_ASSERT(x, ...) { if(!(x)) { LOG_FATAL("Assertion Failed: {0}", __VA_ARGS__); abort(); } }
