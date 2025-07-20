#include "logger.h"

// Статическая переменная для логгераl
std::shared_ptr<spdlog::logger> Logger::logger = nullptr;

void Logger::init(const std::string& filename, const std::string& level_str) {
    try {
        // Инициализация пула потоков для асинхронного логгера (размер очереди 8192, 1 поток)
        spdlog::init_thread_pool(8192, 1);

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);

        logger = std::make_shared<spdlog::async_logger>(
                "pgw_logger",
                file_sink,
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::block  // блокировать основной поток при переполнении очереди
        );

        spdlog::register_logger(logger);

        spdlog::level::level_enum level = parse_level(level_str);
        logger->set_level(level);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

        std::cout << "Введён уровень: " << level_str << std::endl;
        auto level_sv = spdlog::level::to_string_view(level);
        std::string level_sv_str(level_sv.data(), level_sv.size());
        std::cout << "Установлен уровень логгирования: " << level_sv_str << std::endl;

    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Log init failed: " << ex.what() << std::endl;
    }
}

std::shared_ptr<spdlog::logger> Logger::get() {
    if (!logger) {
        std::cout << "Logger not initialized, using default parameters." << std::endl;
        init("pgw.log", "info");
    }
    return logger;
}

spdlog::level::level_enum Logger::parse_level(const std::string& level_str) {
    static const std::map<std::string, spdlog::level::level_enum> level_map = {
            {"trace", spdlog::level::trace},
            {"debug", spdlog::level::debug},
            {"info", spdlog::level::info},
            {"warn", spdlog::level::warn},
            {"error", spdlog::level::err},
            {"critical", spdlog::level::critical},
            {"off", spdlog::level::off}
    };

    std::string input = level_str.empty() ? "info" : level_str;

    auto it = level_map.find(input);
    if (it != level_map.end()) {
        return it->second;
    }

    std::cerr << "Неверный уровень логгирования: \"" << input << "\". Поддерживаются только уровни: ";
    for (const auto& pair : level_map) {
        std::cerr << pair.first << " ";
    }
    std::cerr << std::endl;

    return spdlog::level::info;  // По умолчанию
}
