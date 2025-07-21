#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <unordered_set>


// Функция для конвертации BCD в ASCII для IMSI
std::string bcd2ascii(const std::vector<uint8_t>& bcd);
// Функция для проверки IMSI на наличие в чёрном списке
bool check_imsi(const std::string& imsi_ascii, std::unordered_set<std::string>& black_list);