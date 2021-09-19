set(TEST ${PROJECT_NAME}_tst)

add_executable(${TEST} main.cpp test_utils.cpp test_statements.cpp test_resolver.cpp test_utils.hpp)
set_target_properties(${TEST} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
target_link_libraries(${TEST} PRIVATE gtest ${PROJECT_NAME})

add_test(NAME ${TEST} COMMAND ${TEST} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
