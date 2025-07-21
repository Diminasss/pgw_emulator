#include "ascii2bcd.h"


std::vector<uint8_t> ascii2bcd(std::string ascii) {
    std::vector<uint8_t> result;

    // Проверка, является ли символ числом
    for (char c : ascii) {
        if (not(c >= '0' && c <= '9')) {
            std::cerr << "Символ в строке IMSI не является числом";
            Logger::get()->error("IMSI {} contains not numeral symbols", ascii);
            throw std::invalid_argument("IMSI " + ascii + " contains not numeral symbols");
        }
    }

    // Если нечетное количество, добавляем F в конец
    if (ascii.length() % 2 == 1) {
        ascii += "F";
    }

    // Строка всегда чётной длины
    for (int i = 0; i < ascii.length(); i += 2) {
        uint8_t high = (ascii[i] == 'F') ? 0xF : (ascii[i] - '0');
        uint8_t low = (ascii[i+1] == 'F') ? 0xF : (ascii[i+1] - '0');

        // Добавление двух 4 битных в 1 байт
        result.push_back((high << 4) | low);
    }

    return result;
}
