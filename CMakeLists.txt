cmake_minimum_required(VERSION 3.20)
project(dot_parser)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib/lexy)
add_subdirectory(src)

# testing dependency
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(DOT_PARSER_BUILD_TESTS "whether or not tests should be built" ON)
    if (DOT_PARSER_BUILD_TESTS)
        include(CTest)
        enable_testing()
        add_subdirectory(lib/googletest)
        add_subdirectory(test)
    endif ()
endif()