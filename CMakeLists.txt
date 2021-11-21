cmake_minimum_required(VERSION 3.10)

project(tsprintf LANGUAGES CXX)

add_subdirectory(doctest)

enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(runtest runtest.cpp)
target_include_directories(runtest PUBLIC doctest/doctest)

add_test(NAME runtest COMMAND runtest)
