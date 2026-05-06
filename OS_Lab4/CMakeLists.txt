cmake_minimum_required(VERSION 3.16)

project(ProcessSynchronizationLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Receiver receiver.cpp)
add_executable(Sender sender.cpp)