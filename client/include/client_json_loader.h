#pragma once
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <nlohmann/json.hpp>

class client_json_loader {
public:
    std::string server_ip;
    int server_port;
    std::string log_file;
    std::string log_level;

    void load(const std::string& path);
};
