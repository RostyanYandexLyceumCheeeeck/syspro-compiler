cmake_minimum_required(VERSION 3.14)
project(LexerProject CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


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


add_executable(lexer
    main.cpp
    Lexer.cpp
)

target_include_directories(lexer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(lexer PRIVATE
    ctre::ctre
    nlohmann_json::nlohmann_json
)
