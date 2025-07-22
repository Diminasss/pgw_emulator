#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <cstdio>

#include "cdr_logger.h"

class CDRLoggerTest : public ::testing::Test {
protected:
    std::string test_filename = "test_cdr.csv";

    void SetUp() override {
        std::remove(test_filename.c_str());
        CDRLogger::init(test_filename);
    }

    void TearDown() override {
        std::remove(test_filename.c_str());
    }

    std::vector<std::string> read_file_lines() {
        std::ifstream file(test_filename);
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        return lines;
    }

    void flush_and_wait() {
        CDRLogger::get()->flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
};

TEST_F(CDRLoggerTest, BasicLoggingTest) {
    CDRLogger::write_cdr("123456789012345", CDRAction::SESSION_CREATE);

    flush_and_wait();

    std::vector<std::string> lines = read_file_lines();

    ASSERT_FALSE(lines.empty());
    EXPECT_EQ(lines[0], "timestamp,imsi,action,additional_info");
    EXPECT_GE(lines.size(), 2);
    EXPECT_NE(lines[1].find("123456789012345"), std::string::npos);
    EXPECT_NE(lines[1].find("CREATE"), std::string::npos);
}


