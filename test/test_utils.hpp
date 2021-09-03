#ifndef DOT_PARSER_TEST_UTILS_HPP
#define DOT_PARSER_TEST_UTILS_HPP

#include "parser.hpp"
#include "resolver.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <gtest/gtest.h>

void print_attr_list(std::ostream& os, const dot_parser::detail::attr_list_type& attrs);

void parse_node_stmt_impl(std::ostream& os, const dot_parser::detail::node_stmt_v& v);
void parse_node_stmt(std::ostream& os, const std::string& inp);

void parse_edge_stmt_impl(std::ostream& os, const dot_parser::detail::edge_stmt_v& v, size_t indent=0);
void parse_edge_stmt(std::ostream& os, const std::string& inp);

void parse_item_stmt_impl(std::ostream& os, const std::pair<std::string, std::string>& v);
void parse_item_stmt(std::ostream& os, const std::string& inp);

void parse_attr_stmt_impl(std::ostream& os, const dot_parser::detail::attr_stmt_v& v);
void parse_attr_stmt(std::ostream& os, const std::string& inp);

void parse_stmt_impl(std::ostream& os, const std::vector<dot_parser::detail::stmt_v>& res, size_t indent);
void parse_stmt(std::ostream& os, const std::string& inp);

void parse_dot_graph_impl(std::ostream& os, const dot_parser::dot_graph_raw& res);
void parse_dot_graph(std::ostream& os, const std::string& inp);

void parse_resolved_impl(std::ostream& os, const dot_parser::dot_graph_resolved& resolved, size_t indent);

template<typename F>
void compare(const std::string& in, const std::string& out, F pf) {
    std::stringstream ss;
    pf(ss, in);
    ASSERT_EQ(ss.str(), out);
}

#endif //DOT_PARSER_TEST_UTILS_HPP
