#include "include/json_loader.h"
#include "logger.h"
#include <iostream>
// Сделать логгер
using std::cout;
using std::endl;

int main() {
    // Загрузка JSON конфигурации (файл server_config.json)
    json_loader jsonLoader;
    jsonLoader.load("config/server_config.json");
#ifdef NDEBUG
    Logger::init(jsonLoader.log_file, jsonLoader.log_level);
#else
    Logger::init("/home/diminas/CLionProjects/pgw_emulator/server/logs/pgw.log", "error");
#endif
    // Проба логгера
    Logger::get()->info("Logger initialized");
    Logger::get()->warn("Warn");
    Logger::get()->error("Error");
    return 0;
}
