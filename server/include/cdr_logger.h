#pragma once

#include "logger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <string>
#include <chrono>
#include <memory>
#include <iomanip>
#include <sstream>
#include <iostream>

// Типы действий для CDR
enum class CDRAction {
    SESSION_CREATE,
    SESSION_DELETE,
    SESSION_UPDATE,
    SESSION_EXPIRE,
    GRACEFUL_SHUTDOWN_START,
    GRACEFUL_SHUTDOWN_END
};

class CDRLogger {
public:
    static std::shared_ptr<spdlog::logger> get();
    static void init(const std::string& filename = "pgw_cdr.csv");
    static void write_cdr(const std::string& imsi, CDRAction action);
    static void write_cdr(const std::string& imsi, CDRAction action, const std::string& additional_info);

private:
    static std::shared_ptr<spdlog::logger> cdr_logger;
    static std::string action_to_string(CDRAction action);
    static std::string get_timestamp();
};