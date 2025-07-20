#pragma once
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

class json_loader {
public:
    std::string udp_ip;
    int udp_port;
    int buffer_size;
    int session_timeout_sec;
    std::string cdr_file;
    int http_port;
    int graceful_shutdown_rate;
    std::string log_file;
    std::string log_level;
    std::vector<std::string> blacklist;

    void load(const std::string& path);
};
