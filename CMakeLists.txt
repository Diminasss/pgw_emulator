cmake_minimum_required(VERSION 3.31)
project(pgw_emulator)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(server)
add_subdirectory(client)
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    # Добавляем подкаталог с тестами
    add_subdirectory(server/test)
endif()