set(TEST ${CMAKE_PROJECT_NAME}_tst)

add_executable(${TEST} main.cpp test_utils.h test_statements.cpp)

add_test(NAME ${TEST} COMMAND ${TEST})
target_link_libraries(${TEST} gtest ${CMAKE_PROJECT_NAME})
target_include_directories(${TEST} PRIVATE ../lib/lexy/include ../src )
