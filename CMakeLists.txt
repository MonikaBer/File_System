cmake_minimum_required(VERSION 3.5.1)

project(simplefs)

set(CMAKE_CXX_STANDARD 14)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
enable_testing()


add_subdirectory(src)
add_subdirectory(tests)