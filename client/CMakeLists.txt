cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 23)

# Добавление исполняемых файлов в client
add_executable(client client_main.cpp
        src/ascii2bcd.cpp
        src/logger.cpp)

# Установка библиотек
include(FetchContent)
#spdlog
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.15.3
)
FetchContent_MakeAvailable(spdlog)
target_link_libraries(client PRIVATE spdlog::spdlog)

# Объявление директории с включаемыми файлами
target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)