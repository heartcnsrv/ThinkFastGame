cmake_minimum_required(VERSION 3.14)
project(ThinkFast VERSION 3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

set(SOURCES
    main.cpp
    src/core/GameTypes.cpp
    src/core/WordValidator.cpp
    src/core/AuthManager.cpp
    src/core/GameEngine.cpp
    src/ui/ConsoleUI.cpp
    src/ui/Screens.cpp
    src/utils/CSVManager.cpp
    src/utils/BotNames.cpp
)

add_executable(ThinkFast ${SOURCES})

target_include_directories(ThinkFast PRIVATE ${CMAKE_SOURCE_DIR})

if (MSVC)
    target_compile_options(ThinkFast PRIVATE /W4)
else()
    target_compile_options(ThinkFast PRIVATE
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion
        -O2
    )
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
    AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(ThinkFast PRIVATE stdc++fs)
endif()

message(STATUS "ThinkFast v${PROJECT_VERSION} configured.")
message(STATUS "  Sources : ${SOURCES}")
message(STATUS "  Standard: C++${CMAKE_CXX_STANDARD}")
