#include "cdr_logger.h"


// Статическая переменная для CDR логгера
std::shared_ptr<spdlog::logger> CDRLogger::cdr_logger = nullptr;

void CDRLogger::init(const std::string& filename) {
    if (cdr_logger){
        Logger::get()->error("CDRLogger: CDRLogger has already initialised");
        throw std::runtime_error("CDRLogger has already initialised");
    }
    if (filename.empty()){
        Logger::get()->error("CDRLogger: Empty file path argument");
        throw std::invalid_argument("CDRLogger: Empty file path argument");
    }
    try {
        // Используем тот же thread pool что и основной логгер
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);

        cdr_logger = std::make_shared<spdlog::async_logger>(
                "cdr_logger",
                file_sink,
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::block
        );

        // Регистрируем CDR логгер
        spdlog::register_logger(cdr_logger);

        // Устанавливаем простой формат: только сообщение без дополнительной информации
        cdr_logger->set_pattern("%v");
        cdr_logger->set_level(spdlog::level::info);

        // Записываем заголовок CSV файла
        cdr_logger->info("timestamp,imsi,action,additional_info");

    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "CDR Logger init failed: " << ex.what() << std::endl;
    }
}

std::shared_ptr<spdlog::logger> CDRLogger::get() {
    if (!cdr_logger) {
        Logger::get()->error("CDRLogger: CDR Logger not initialized");
        throw std::logic_error("CDRLogger: CDRLogger not initialized. You cant get it");
    }
    return cdr_logger;
}

void CDRLogger::write_cdr(const std::string& imsi, CDRAction action) {
    write_cdr(imsi, action, "");
}

void CDRLogger::write_cdr(const std::string& imsi, CDRAction action, const std::string& additional_info) {
    if (!cdr_logger) {
        init();
    }

    std::string timestamp = get_timestamp();
    std::string action_str = action_to_string(action);

    // Формируем CSV строку
    std::ostringstream oss;
    oss << timestamp << "," << imsi << "," << action_str;
    if (!additional_info.empty()) {
        oss << "," << additional_info;
    } else {
        oss << ",";
    }

    cdr_logger->info(oss.str());
}

std::string CDRLogger::action_to_string(CDRAction action) {
    switch (action) {
        case CDRAction::SESSION_CREATE:
            return "CREATE";
        case CDRAction::SESSION_DELETE:
            return "DELETE";
        case CDRAction::SESSION_UPDATE:
            return "UPDATE";
        case CDRAction::SESSION_EXPIRE:
            return "EXPIRE";
        case CDRAction::GRACEFUL_SHUTDOWN_START:
            return "GRACEFUL_START";
        case CDRAction::GRACEFUL_SHUTDOWN_END:
            return "GRACEFUL_END";
        default:
            return "UNKNOWN";
    }
}

std::string CDRLogger::get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d_%H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();

    return oss.str();
}