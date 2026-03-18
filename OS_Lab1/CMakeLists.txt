cmake_minimum_required(VERSION 3.10)

project(Lab1Processes)

set(CMAKE_CXX_STANDARD 98)

# Общие заголовки
include_directories(common)

# Creator
add_executable(creator
    creator/creator.cpp
)

# Reporter
add_executable(reporter
    reporter/reporter.cpp
)

# Main
add_executable(main_app
    main/main.cpp
)