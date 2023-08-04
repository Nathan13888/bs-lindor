cmake_minimum_required(VERSION 3.25)
project(supervisor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS -static)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

add_executable(supervisor src/main.cpp src/utils.cpp src/Minimax.h src/Minimax.cpp src/utils.h)

target_link_libraries(supervisor PUBLIC nlohmann_json::nlohmann_json)

add_subdirectory(include/game)
