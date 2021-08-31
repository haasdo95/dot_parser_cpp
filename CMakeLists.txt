cmake_minimum_required(VERSION 3.20)
project(dot_parser)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib/lexy)

add_subdirectory(src)

add_subdirectory(lib/googletest)
add_subdirectory(test)
