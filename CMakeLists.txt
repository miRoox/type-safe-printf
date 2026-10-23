cmake_minimum_required(VERSION 4.4)

project(tsprintf LANGUAGES CXX)

add_library(tsprintf INTERFACE)
add_library(tsprintf::tsprintf ALIAS tsprintf)
target_include_directories(tsprintf INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(tsprintf INTERFACE cxx_std_14)

include(CTest)

if(BUILD_TESTING)
    set(DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF)
    set(DOCTEST_NO_INSTALL ON)
    add_subdirectory(doctest EXCLUDE_FROM_ALL)

    add_executable(runtest runtest.cpp)
    target_link_libraries(runtest PRIVATE tsprintf::tsprintf doctest::doctest)
    set_target_properties(runtest PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    add_test(NAME runtest COMMAND runtest)
endif()
