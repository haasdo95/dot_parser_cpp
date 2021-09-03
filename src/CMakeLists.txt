add_library(dot_parser parser.hpp non_terminals.hpp resolver.hpp non_terminals.cpp resolver.cpp parser.cpp)
set_target_properties(dot_parser PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(dot_parser SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dot_parser PRIVATE lexy)
