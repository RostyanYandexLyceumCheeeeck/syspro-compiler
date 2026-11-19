cmake_minimum_required(VERSION 3.14)
project(Lexer CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()


include(FetchContent)

if(NOT TARGET ctre::ctre)
    FetchContent_Declare(
    ctre
    GIT_REPOSITORY "https://github.com/hanickadot/compile-time-regular-expressions"
    GIT_TAG        v3.11.0
    )
    FetchContent_MakeAvailable(ctre)
endif()

if(NOT TARGET nlohmann_json::nlohmann_json)
    FetchContent_Declare(
        json
        GIT_REPOSITORY "https://github.com/nlohmann/json"
        GIT_TAG v3.11.3
    )
    FetchContent_MakeAvailable(json)
endif()


set(JSON_BuildTests OFF CACHE INTERNAL "")

add_executable(lexer
    main_lexer.cpp
    Lexer.cpp
)

target_include_directories(lexer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(lexer PRIVATE
    ctre::ctre
    nlohmann_json::nlohmann_json
)