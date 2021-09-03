set(TEST ${CMAKE_PROJECT_NAME}_tst)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_executable(${TEST} main.cpp test_utils.cpp test_statements.cpp test_resolver.cpp test_utils.hpp)
target_link_libraries(${TEST} PRIVATE gtest ${CMAKE_PROJECT_NAME} lexy)

add_test(NAME ${TEST} COMMAND ${TEST})
