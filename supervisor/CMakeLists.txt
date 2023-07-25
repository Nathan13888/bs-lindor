cmake_minimum_required(VERSION 3.25)
project(supervisor)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

add_executable(supervisor src/main.cpp src/utils.cpp src/Minimax.h src/Minimax.cpp src/utils.h)
add_custom_command(TARGET supervisor
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:supervisor> SomeOtherDir)

target_link_libraries(supervisor PUBLIC nlohmann_json::nlohmann_json)

add_subdirectory(include/game)
