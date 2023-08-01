#include "logger.h"

std::shared_ptr<spdlog::async_logger> Logger::logger;

void Logger::init()
{
    spdlog::init_thread_pool(8192, 1);
    std::vector<spdlog::sink_ptr> logSinks;
    logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Maya.log", false));

    logSinks[0]->set_pattern("%^[%Y-%m-%d] [%T] [%l]: %v%$");
    logSinks[1]->set_pattern("[%Y-%m-%d] [%T] [%l]: %v");

    logger = std::make_shared<spdlog::async_logger>("Maya Logger", begin(logSinks), end(logSinks), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    spdlog::register_logger(logger);
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);
}

std::shared_ptr<spdlog::async_logger>& Logger::getLogger() {
    return logger;
}