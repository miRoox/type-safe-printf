cmake_minimum_required(VERSION 3.10)

project(tsprintf LANGUAGES CXX)

add_subdirectory(doctest)

enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(good good.cpp)
target_include_directories(good PUBLIC doctest/doctest)

add_test(NAME good COMMAND good)
