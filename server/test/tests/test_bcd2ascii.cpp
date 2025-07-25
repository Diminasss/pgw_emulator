#include <gtest/gtest.h>
#include <unordered_set>
#include "bcd2ascii.h"

// Тест на корректную конвертацию BCD → ASCII
TEST(BCD2AsciiTest, ConvertsCorrectly) {
    std::vector<uint8_t> bcd = {0x21, 0x43, 0x65, 0x78, 0x9F};
    std::string result = bcd2ascii(bcd);
    EXPECT_EQ(result, "214365789");
}
TEST(BCD2AsciiTest, ConvertsEvenCorrectly) {
    std::vector<uint8_t> bcd = {0x21, 0x43, 0x65, 0x78};
    std::string result = bcd2ascii(bcd);
    EXPECT_EQ(result, "21436578");
}
TEST(BCD2AsciiTest, HandlesPaddingFNibble) {
    std::vector<uint8_t> bcd = {0x21, 0x4F};
    std::string result = bcd2ascii(bcd);
    EXPECT_EQ(result, "214");
}

TEST(BCD2AsciiTest, StopsOnInvalidSecondNibble) {
    std::vector<uint8_t> bcd = {0x2A};  // low nibble = 0xA (invalid)

    EXPECT_THROW(bcd2ascii(bcd), std::runtime_error);
}
TEST(BCD2AsciiTest, StopsOnInvalidFirstNibble) {
    std::vector<uint8_t> bcd = {0xA2};  // low nibble = 0xA (invalid)
    EXPECT_THROW(bcd2ascii(bcd), std::runtime_error);
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

TEST(CheckImsiTest, ReturnsFalseIfSizeOver15) {
    std::unordered_set<std::string> blacklist;
    std::string fake_imsi(16, '1'); // Строка из 16 единиц
    EXPECT_FALSE(check_imsi(fake_imsi, blacklist));
}

TEST(CheckImsiTest, ReturnsFalseIfSize15) {
    std::unordered_set<std::string> blacklist;
    std::string fake_imsi(15, '1'); // Строка из 15 единиц
    EXPECT_TRUE(check_imsi(fake_imsi, blacklist));
}