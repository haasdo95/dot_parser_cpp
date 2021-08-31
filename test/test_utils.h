#ifndef DOT_PARSER_TEST_UTILS_H
#define DOT_PARSER_TEST_UTILS_H

#include "parser.hpp"
#include <lexy/input/string_input.hpp>
#include <lexy/action/parse.hpp> // lexy::parse
#include <lexy_ext/report_error.hpp>
#include <iostream>

void print_attr_list(std::ostream& os, const dot_parser::attr_list_type& attrs) {
    for (int i = 0; i < attrs.size(); ++i) {
        os << attrs[i].first << "=" << attrs[i].second;
        if (i!=attrs.size()-1) {
            os << ", ";
        }
    }
}

void parse_node_stmt_impl(std::ostream& os, const dot_parser::node_stmt_v& v) {
    os << v.node_name;
    if (!v.attrs.empty()) {
        os << " [";
        print_attr_list(os, v.attrs);
        os << "]";
    }
    os << '\n';
}
void parse_node_stmt(std::ostream& os, const std::string& inp) {
    auto txt = lexy::string_input(inp.c_str(), inp.size());
    auto res = lexy::parse<dot_parser::node_stmt>(txt, lexy_ext::report_error).value();
    parse_node_stmt_impl(os, res);
}

void parse_edge_stmt_impl(std::ostream& os, const dot_parser::edge_stmt_v& v, size_t indent=0) {
    const auto& edges = v.edges;
    for (size_t i = 0; i < edges.size(); ++i) {
        const auto& edge = edges[i];
        if (i!=0) {  // the first should be taken care of by outer
            for (int j = 0; j < indent; ++j) {
                os << "\t";
            }
        }
        os << edge.src << edge.edge_op << edge.tgt;
        if (!v.attrs.empty()) {
            os << " [";
            print_attr_list(os, v.attrs);
            os << "]";
        }
        os << '\n';
    }
}
void parse_edge_stmt(std::ostream& os, const std::string& inp) {
    auto txt = lexy::string_input(inp.c_str(), inp.size());
    auto res = lexy::parse<dot_parser::edge_stmt_branch>(txt, lexy_ext::report_error).value();
    parse_edge_stmt_impl(os, res);
}

void parse_item_stmt_impl(std::ostream& os, const std::pair<std::string, std::string>& v) {
    os << v.first << "=" << v.second << '\n';
}
void parse_item_stmt(std::ostream& os, const std::string& inp) {
    auto txt = lexy::string_input(inp.c_str(), inp.size());
    auto res = lexy::parse<dot_parser::attr_item>(txt, lexy_ext::report_error).value();
    parse_item_stmt_impl(os, res);
}

void parse_attr_stmt_impl(std::ostream& os, const dot_parser::attr_stmt_v& v) {
    ASSERT_FALSE(v.attrs.empty());
    os << v.type << " [";
    print_attr_list(os, v.attrs);
    os << "]\n";
}
void parse_attr_stmt(std::ostream& os, const std::string& inp) {
    auto txt = lexy::string_input(inp.c_str(), inp.size());
    auto res = lexy::parse<dot_parser::attr_stmt>(txt, lexy_ext::report_error).value();
    parse_attr_stmt_impl(os, res);
}


void parse_stmt_impl(std::ostream& os, const std::vector<dot_parser::stmt_v>& res, size_t indent) {
    for (const auto& r: res) {
        for (size_t i = 0; i < indent; ++i) {
            os << "\t";
        }
        if (std::holds_alternative<dot_parser::node_stmt_v>(r.val)) {
            auto v = std::get<dot_parser::node_stmt_v>(r.val);
            parse_node_stmt_impl(os, v);
        } else if (std::holds_alternative<dot_parser::edge_stmt_v>(r.val)) {
            auto v = std::get<dot_parser::edge_stmt_v>(r.val);
            parse_edge_stmt_impl(os, v, indent);
        } else if (std::holds_alternative<dot_parser::attr_stmt_v>(r.val)) {
            auto v = std::get<dot_parser::attr_stmt_v>(r.val);
            parse_attr_stmt_impl(os, v);
        } else if (std::holds_alternative<dot_parser::attr_item_v>(r.val)) {
            auto v = std::get<dot_parser::attr_item_v>(r.val);
            parse_item_stmt_impl(os, v);
        } else {  // subgraph
            if (!r.name.empty()) {
                os << r.name << " ";
            }
            auto v = std::get<std::vector<dot_parser::stmt_v>>(r.val);
            os << "{\n";
            parse_stmt_impl(os, v, indent+1);
            for (size_t i = 0; i < indent; ++i) {
                os << "\t";
            }
            os << "}\n";
        }
    }
}
void parse_stmt(std::ostream& os, const std::string& inp) {
    auto txt = lexy::string_input(inp.c_str(), inp.size());
    auto res = lexy::parse<dot_parser::statement_list>(txt, lexy_ext::report_error).value();
    parse_stmt_impl(os, res, 0);
}

void parse_dot_graph(std::ostream& os, const std::string& inp) {
    auto txt = lexy::string_input(inp.c_str(), inp.size());
    auto res = lexy::parse<dot_parser::dot_graph>(txt, lexy_ext::report_error).value();
    if (res.is_strict) {
        os << "strict ";
    }
    os << res.graph_type << " ";
    if (!res.name.empty()) {
        os << res.name << " ";
    }
    os << "{\n";
    parse_stmt_impl(os, res.statements, 1);
    os << "}\n";
}

template<typename F>
void compare(const std::string& in, const std::string& out, F pf) {
    std::stringstream ss;
    pf(ss, in);
    ASSERT_EQ(ss.str(), out);
}

#endif //DOT_PARSER_TEST_UTILS_H
