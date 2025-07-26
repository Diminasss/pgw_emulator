#include "json_loader.h"
#include <gtest/gtest.h>
#include <fstream>

TEST(JsonLoaderTest, LoadValidConfig) {
    // Создаём временный файл конфигурации JSON
    const std::string filename = "test_config.json";
    std::ofstream ofs(filename);
    ofs << R"({
        "udp_ip": "127.0.0.1",
        "buffer_size": 512,
        "udp_port": 9000,
        "session_timeout_sec": 60,
        "cdr_file": "test_cdr.log",
        "http_port": 8000,
        "graceful_shutdown_rate_sec": 1,
        "graceful_shutdown_batch_size": 2,
        "log_file": "test.log",
        "log_level": "DEBUG",
        "max_events": 1024,
        "blacklist": ["111111111111111", "222222222222222"]
    })";
    ofs.close();
    json_loader loader;
    EXPECT_NO_THROW(loader.load(filename));
    EXPECT_EQ(loader.udp_ip, "127.0.0.1");
    EXPECT_EQ(loader.buffer_size, 512);
    EXPECT_EQ(loader.udp_port, 9000);
    EXPECT_EQ(loader.session_timeout_sec, 60);
    EXPECT_EQ(loader.cdr_file, "test_cdr.log");
    EXPECT_EQ(loader.http_port, 8000);
    EXPECT_EQ(loader.graceful_shutdown_rate_sec, 1);
    EXPECT_EQ(loader.graceful_shutdown_batch_size, 2);
    EXPECT_EQ(loader.log_file, "test.log");
    EXPECT_EQ(loader.log_level, "DEBUG");
    EXPECT_EQ(loader.max_events, 1024);
    EXPECT_TRUE(loader.blacklist.find("111111111111111") != loader.blacklist.end());
    EXPECT_TRUE(loader.blacklist.find("222222222222222") != loader.blacklist.end());
}

TEST(JsonLoaderTest, FileNotFound) {
    json_loader loader;
    EXPECT_THROW(loader.load("nonexistent.json"), std::runtime_error);
}

