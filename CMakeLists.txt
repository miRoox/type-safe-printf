cmake_minimum_required(VERSION 3.5)

project(tsprintf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tsprintf test.cpp)

target_compile_definitions(tsprintf PRIVATE ok)
