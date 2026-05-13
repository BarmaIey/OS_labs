cmake_minimum_required(VERSION 3.16)

project(NamedPipesLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Server server.cpp)
add_executable(Client client.cpp)