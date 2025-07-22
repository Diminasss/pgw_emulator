#include "bcd2ascii.h"

// Функция для конвертации BCD в ASCII для IMSI
std::string bcd2ascii(const std::vector<uint8_t>& bcd) {
    std::string result;
    size_t len = bcd.size();

    for (size_t i = 0; i < len; ++i) {
        uint8_t byte = bcd[i];

        // Сначала старший ниббл
        uint8_t high = (byte & 0xF0) >> 4;
        uint8_t low  = byte & 0x0F;

        // Обработка старшего ниббла
        if (high <= 9) {
            result += static_cast<char>('0' + high);
        } else if (high == 0xF) {
            break; // padding — конец строки
        } else {
            break; // недопустимый BCD
        }

        // Обработка младшего ниббла
        if (low <= 9) {
            result += static_cast<char>('0' + low);
        } else if (low == 0xF) {
            break; // padding — конец строки
        } else {
            break; // недопустимый BCD
        }
    }

    return result;
}

bool check_imsi(const std::string& imsi_ascii, std::unordered_set<std::string>& black_list) {
    if (black_list.contains(imsi_ascii)){
        return false;
    }
    if (imsi_ascii.size() > 15){
        return false;
    }
    return true;
}