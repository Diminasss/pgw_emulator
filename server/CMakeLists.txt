cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 23)

add_executable(server server_main.cpp
        include/json_loader.h
        src/json_loader.cpp
        include/logger.h
        src/logger.cpp)

include(FetchContent)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.15.3
)

FetchContent_MakeAvailable(spdlog)

target_link_libraries(server PRIVATE spdlog::spdlog)


FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(json)

target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(server PRIVATE nlohmann_json::nlohmann_json)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/config/server_config.json
        ${CMAKE_CURRENT_BINARY_DIR}/config/server_config.json
        COPYONLY
)