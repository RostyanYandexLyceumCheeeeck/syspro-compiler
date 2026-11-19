cmake_minimum_required(VERSION 3.14)
project(SysProCompiler CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

option(BUILD_LEXER "Build lexer executable" ON)
option(BUILD_PARSER "Build parser executable" ON)


include(FetchContent)

FetchContent_Declare(
  ctre
  GIT_REPOSITORY "https://github.com/hanickadot/compile-time-regular-expressions"
  GIT_TAG        v3.11.0
)

FetchContent_Declare(
    json
    GIT_REPOSITORY "https://github.com/nlohmann/json"
    GIT_TAG v3.11.3
)

set(JSON_BuildTests OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(ctre json)


if(BUILD_LEXER AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lexer/CMakeLists.txt")
    add_subdirectory(lexer)
endif()

if(BUILD_PARSER AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/parser/CMakeLists.txt")
    add_subdirectory(parser)
endif()