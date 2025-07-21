#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <string>
#include <map>
#include <iostream>

class Logger {
public:
    static std::shared_ptr<spdlog::logger> get();
    static void init(const std::string& filename, const std::string& level_str);

private:
    static std::shared_ptr<spdlog::logger> logger;
    static spdlog::level::level_enum parse_level(const std::string& level_str);
};