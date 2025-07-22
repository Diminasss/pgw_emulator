#include <gtest/gtest.h>
#include "logger.h"

int main(int argc, char **argv) {
    // Инициализируем Google Test
    ::testing::InitGoogleTest(&argc, argv);

    // Инициализируем логгер для тестов (чтобы избежать ошибок)
    Logger::init("test.log", "debug");

    // Запускаем все тесты
    return RUN_ALL_TESTS();
}
