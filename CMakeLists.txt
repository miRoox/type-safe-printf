cmake_minimum_required(VERSION 3.10)

project(tsprintf LANGUAGES CXX)

# The bundled doctest declares a CMake 3.0 policy version, which CMake 4 removed.
# Use this project's policy baseline for it while preserving higher user overrides.
if(CMAKE_VERSION VERSION_GREATER_EQUAL 4.0 AND
   (NOT DEFINED CMAKE_POLICY_VERSION_MINIMUM OR
    CMAKE_POLICY_VERSION_MINIMUM VERSION_LESS 3.10))
    set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
endif()

add_subdirectory(doctest)

enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(runtest runtest.cpp)
target_include_directories(runtest PUBLIC doctest/doctest)

add_test(NAME runtest COMMAND runtest)
