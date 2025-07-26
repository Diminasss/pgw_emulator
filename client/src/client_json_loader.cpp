#include "client_json_loader.h"

void client_json_loader::load(const std::string &path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Could not open config file");
    }

    nlohmann::json config;
    file >> config;

    try {
        server_ip = config.at("server_ip").get<std::string>();
        server_port = config.at("server_port").get<int>();
        log_file = config.at("log_file").get<std::string>();
        log_level = config.at("log_level").get<std::string>();
    } catch (const std::exception& e) {
        std::cerr << "Error parsing config: " << e.what() << "\n";
    }
}
