#ifndef DOT_PARSER_TEST_UTILS_CPP
#define DOT_PARSER_TEST_UTILS_CPP

#include "test_utils.hpp"
#include <lexy/input/string_input.hpp>
#include <lexy/action/parse.hpp> // lexy::parse
#include <lexy_ext/report_error.hpp>
#include <iostream>

void add_indent(std::ostream& os, size_t indent) {
    for (size_t i = 0; i < indent; ++i) {
        os << "\t";
    }
}

void print_attr_list(std::ostream& os, const dot_parser::detail::attr_list_type& attrs) {
    for (int i = 0; i < attrs.size(); ++i) {
        os << attrs[i].first << "=" << attrs[i].second;
        if (i!=attrs.size()-1) {
            os << ", ";
        }
    }
}

void parse_node_stmt_impl(std::ostream& os, const dot_parser::detail::node_stmt_v& v) {
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
    auto res = lexy::parse<dot_parser::parsing::node_stmt>(txt, lexy_ext::report_error).value();
    parse_node_stmt_impl(os, res);
}

void parse_edge_stmt_impl(std::ostream& os, const dot_parser::detail::edge_stmt_v& v, size_t indent) {
    const auto& edges = v.edges;
    for (size_t i = 0; i < edges.size(); ++i) {
        const auto& edge = edges[i];
        if (i!=0) {  // the first should be taken care of by outer
            add_indent(os, indent);
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
    auto res = lexy::parse<dot_parser::parsing::edge_stmt_branch>(txt, lexy_ext::report_error).value();
    parse_edge_stmt_impl(os, res);
}

void parse_item_stmt_impl(std::ostream& os, const std::pair<std::string, std::string>& v) {
    os << v.first << "=" << v.second << '\n';
}
void parse_item_stmt(std::ostream& os, const std::string& inp) {
    auto txt = lexy::string_input(inp.c_str(), inp.size());
    auto res = lexy::parse<dot_parser::parsing::attr_item>(txt, lexy_ext::report_error).value();
    parse_item_stmt_impl(os, res);
}

void parse_attr_stmt_impl(std::ostream& os, const dot_parser::detail::attr_stmt_v& v) {
    os << v.type << " [";
    print_attr_list(os, v.attrs);
    os << "]\n";
}
void parse_attr_stmt(std::ostream& os, const std::string& inp) {
    auto txt = lexy::string_input(inp.c_str(), inp.size());
    auto res = lexy::parse<dot_parser::parsing::attr_stmt>(txt, lexy_ext::report_error).value();
    parse_attr_stmt_impl(os, res);
}

void parse_stmt_impl(std::ostream& os, const std::vector<dot_parser::detail::stmt_v>& res, size_t indent) {
    for (const auto& r: res) {
        add_indent(os, indent);
        if (std::holds_alternative<dot_parser::detail::node_stmt_v>(r.val)) {
            auto v = std::get<dot_parser::detail::node_stmt_v>(r.val);
            parse_node_stmt_impl(os, v);
        } else if (std::holds_alternative<dot_parser::detail::edge_stmt_v>(r.val)) {
            auto v = std::get<dot_parser::detail::edge_stmt_v>(r.val);
            parse_edge_stmt_impl(os, v, indent);
        } else if (std::holds_alternative<dot_parser::detail::attr_stmt_v>(r.val)) {
            auto v = std::get<dot_parser::detail::attr_stmt_v>(r.val);
            parse_attr_stmt_impl(os, v);
        } else if (std::holds_alternative<dot_parser::detail::attr_item_v>(r.val)) {
            auto v = std::get<dot_parser::detail::attr_item_v>(r.val);
            parse_item_stmt_impl(os, v);
        } else {  // subgraph
            if (!r.name.empty()) {
                os << r.name << " ";
            }
            auto v = std::get<std::vector<dot_parser::detail::stmt_v>>(r.val);
            os << "{\n";
            parse_stmt_impl(os, v, indent+1);
            add_indent(os, indent);
            os << "}\n";
        }
    }
}
void parse_stmt(std::ostream& os, const std::string& inp) {
    auto txt = lexy::string_input(inp.c_str(), inp.size());
    auto res = lexy::parse<dot_parser::parsing::statement_list>(txt, lexy_ext::report_error).value();
    parse_stmt_impl(os, res, 0);
}

void parse_resolved_impl(std::ostream& os, const dot_parser::dot_graph_resolved& resolved, size_t indent) {
    // opening curly
    if (!resolved.name.empty()) {
        os << resolved.name << " ";
    }
    os << "{\n";
    // print graph attributes
    dot_parser::detail::attr_list_type attrs{ resolved.graph_attrs.begin(), resolved.graph_attrs.end() };
    add_indent(os, indent+1);
    os << '[';
    print_attr_list(os, attrs);
    os << "]\n";
    // print statements
    for (const auto& stmt: resolved.statements) {
        add_indent(os, indent+1);
        if (std::holds_alternative<dot_parser::detail::node_stmt_v>(stmt)) {
            const auto& v = std::get<dot_parser::detail::node_stmt_v>(stmt);
            parse_node_stmt_impl(os, v);
        } else if (std::holds_alternative<dot_parser::detail::edge_stmt_v>(stmt)) {
            const auto& v = std::get<dot_parser::detail::edge_stmt_v>(stmt);
            parse_edge_stmt_impl(os, v, indent+1);
        } else {
            assert(std::holds_alternative<dot_parser::dot_graph_resolved>(stmt));
            const auto& v = std::get<dot_parser::dot_graph_resolved>(stmt);
            parse_resolved_impl(os, v, indent+1);
        }
    }
    // closing curly
    add_indent(os, indent);
    os << "}\n";
}

void parse_dot_graph_impl(std::ostream& os, const dot_parser::dot_graph_raw& res) {
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

void parse_dot_graph(std::ostream& os, const std::string& inp) {
    auto res = dot_parser::parse(inp);
    parse_dot_graph_impl(os, res);
}



#endif //DOT_PARSER_TEST_UTILS_CPP
