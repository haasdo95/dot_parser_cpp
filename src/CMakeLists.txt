add_library(dot_parser INTERFACE)
target_sources(dot_parser INTERFACE parser.hpp)

set_target_properties(dot_parser PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(dot_parser INTERFACE ../lib/lexy/include)