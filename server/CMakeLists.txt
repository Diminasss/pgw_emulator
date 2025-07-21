cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 23)

# Добавление исполняемых файлов в server
add_executable(server server_main.cpp
        src/json_loader.cpp
        src/logger.cpp
)

# Установка библиотек
include(FetchContent)
# spdlog
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.15.3
)
FetchContent_MakeAvailable(spdlog)
target_link_libraries(server PRIVATE spdlog::spdlog)

#nlohmann json
FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(json)
target_link_libraries(server PRIVATE nlohmann_json::nlohmann_json)

# Копирование json в build папку
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/config/server_config.json
        ${CMAKE_CURRENT_BINARY_DIR}/config/server_config.json
        COPYONLY
)

# Объявление директории с включаемыми файлами
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)