//
// Created by dou on 9/2/21.
//

#ifndef DOT_PARSER_NON_TERMINALS_HPP
#define DOT_PARSER_NON_TERMINALS_HPP

#include <string>
#include <vector>
#include <map>
#include <variant>

namespace dot_parser {
    struct edge {
        std::string src;
        std::string edge_op;
        std::string tgt;
        bool operator==(const edge& other) const;
        [[nodiscard]] std::size_t hash() const;
        [[nodiscard]] std::string to_string() const;
    };
}
// custom hash for edge
namespace std {
    template<>
    struct hash<dot_parser::edge> {
        std::size_t operator()(const dot_parser::edge& e) const {
            return e.hash();
        }
    };
}

// non-terminal data structures
namespace dot_parser::detail {
    using attr_item_v = std::pair<std::string, std::string>;
    using attr_list_type = std::vector<attr_item_v>;

    struct node_stmt_v {
        std::string node_name;
        attr_list_type attrs;
    };

    struct edge_stmt_v {
        std::vector<edge> edges;
        attr_list_type attrs;
    };

    struct attr_stmt_v {
        std::string type;
        attr_list_type attrs;
    };

    struct stmt_v {
        std::string name;
        std::variant<node_stmt_v, edge_stmt_v, attr_stmt_v, attr_item_v, std::vector<stmt_v>> val;
        stmt_v(const node_stmt_v& v): val{v} {};
        stmt_v(const edge_stmt_v& v): val{v} {};
        stmt_v(const std::variant<attr_stmt_v, node_stmt_v, attr_item_v>& v);
        stmt_v(const std::vector<stmt_v>& v): val{v} {};
        stmt_v(std::string name, const std::vector<stmt_v>& v): name{std::move(name)}, val{v} {}
    };
}

namespace dot_parser {
    // raw graph from the parser directly
    struct dot_graph_raw {
        bool is_strict{};
        std::string graph_type;  // graph or digraph
        std::string name;
        std::vector<detail::stmt_v> statements;
    };

    namespace detail {  // used for collecting attributes from outer scope
        struct external_attrs {
            using a_table = std::map<std::string, std::string>;
            a_table graph;
            a_table node;
            a_table edge;
        };
    }
    // processed graph with resolved attribute + some checks
    struct dot_graph_resolved;
    using resolved_stmt_type = std::variant<detail::node_stmt_v, detail::edge_stmt_v, dot_graph_resolved>;

    struct dot_graph_resolved {
        bool is_strict{};
        std::string graph_type;
        std::string name;
        std::map<std::string, std::string> graph_attrs;
        std::vector<resolved_stmt_type> statements;
    };

    // flatten out subgraph statements, discarding sub-graph attributes at the same time
    using flat_stmt_type = std::variant<detail::node_stmt_v, detail::edge_stmt_v>;
    struct dot_graph_flat {
        bool is_strict{};
        std::string graph_type;
        std::vector<flat_stmt_type> statements;
    };

}

#endif //DOT_PARSER_NON_TERMINALS_HPP
