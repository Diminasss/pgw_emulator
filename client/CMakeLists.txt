cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 23)

# Добавление исполняемых файлов в client
add_executable(client client_main.cpp
        src/ascii2bcd.cpp
        src/client_logger.cpp
        src/client_json_loader.cpp
)

# Копирование json в build папку
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/config/client_config.json
        ${CMAKE_CURRENT_BINARY_DIR}/config/client_config.json
        COPYONLY
)

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

#nlohmann json
FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(json)
target_link_libraries(client PRIVATE nlohmann_json::nlohmann_json)

# Объявление директории с включаемыми файлами
target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

