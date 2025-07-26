cmake_minimum_required(VERSION 3.31)

enable_testing()

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG main
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Основные файлы
set(SERVER_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/bcd2ascii.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/pgw_sessions.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/cdr_logger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/logger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/json_loader.cpp
)

# Тестовые файлы
set(TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_bcd2ascii.cpp
        #${CMAKE_CURRENT_SOURCE_DIR}/tests/test_pgw_sessions.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_cdr_logger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_json_loader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_session_manager.cpp
)

add_executable(server_tests ${SERVER_SOURCES} ${TEST_SOURCES})

target_include_directories(server_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(server_tests
        GTest::gtest_main
        GTest::gmock_main
        spdlog
        nlohmann_json::nlohmann_json
)

include(GoogleTest)
gtest_discover_tests(server_tests)
