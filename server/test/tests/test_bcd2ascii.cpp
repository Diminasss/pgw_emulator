#include <gtest/gtest.h>
#include <unordered_set>
#include "bcd2ascii.h"

// Тест на корректную конвертацию BCD → ASCII
TEST(BCD2AsciiTest, ConvertsCorrectly) {
    std::vector<uint8_t> bcd = {0x21, 0x43, 0x65, 0x7F};  // = "1234567"
    std::string result = bcd2ascii(bcd);
    EXPECT_EQ(result, "2143657");
}

TEST(BCD2AsciiTest, HandlesPaddingFNibble) {
    std::vector<uint8_t> bcd = {0x21, 0x4F};  // "12", затем padding
    std::string result = bcd2ascii(bcd);
    EXPECT_EQ(result, "214");
}

TEST(BCD2AsciiTest, StopsOnInvalidNibble) {
    std::vector<uint8_t> bcd = {0x2A};  // low nibble = 0xA (invalid)
    std::string result = bcd2ascii(bcd);
    EXPECT_EQ(result, "2");  // только один символ, потом break
}

TEST(BCD2AsciiTest, ReturnsEmptyForEmptyInput) {
    std::vector<uint8_t> bcd = {};
    std::string result = bcd2ascii(bcd);
    EXPECT_TRUE(result.empty());
}

// Тесты для check_imsi

TEST(CheckImsiTest, ReturnsFalseIfInBlacklist) {
    std::unordered_set<std::string> blacklist = {"250123456789012"};
    EXPECT_FALSE(check_imsi("250123456789012", blacklist));
}

TEST(CheckImsiTest, ReturnsTrueIfNotInBlacklistAndStartsWith250) {
    std::unordered_set<std::string> blacklist = {"250987654321098"};
    EXPECT_TRUE(check_imsi("250123456789012", blacklist));
}

TEST(CheckImsiTest, ReturnsFalseIfNotStartsWith250) {
    std::unordered_set<std::string> blacklist;
    EXPECT_FALSE(check_imsi("123456789012345", blacklist));
}