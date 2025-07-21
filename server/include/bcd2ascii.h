#pragma once

#include <vector>
#include <string>
#include <cstdint>


// Функция для конвертации BCD в ASCII для IMSI
std::string bcd2ascii(const std::vector<uint8_t>& bcd);