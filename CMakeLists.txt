cmake_minimum_required(VERSION 3.5)
project(ferrugo-ansi)

enable_testing()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

add_subdirectory(tests)

include(dependencies.cmake)