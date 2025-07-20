#include "json_loader.h"

void json_loader::load(const std::string &path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Could not open config file");
    }

    nlohmann::json config;
    file >> config;

    try {
        udp_ip = config.at("udp_ip").get<std::string>();
        udp_port = config.at("udp_port").get<int>();
        session_timeout_sec = config.at("session_timeout_sec").get<int>();
        cdr_file = config.at("cdr_file").get<std::string>();
        http_port = config.at("http_port").get<int>();
        graceful_shutdown_rate = config.at("graceful_shutdown_rate").get<int>();
        log_file = config.at("log_file").get<std::string>();
        log_level = config.at("log_level").get<std::string>();
        blacklist = config.at("blacklist").get<std::vector<std::string>>();
    } catch (const std::exception& e) {
        std::cerr << "Error parsing config: " << e.what() << "\n";
        throw;  // дальше кидаем исключение наружу
    }
}
