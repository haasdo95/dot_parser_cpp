#ifndef DOT_PARSER_RESOLVER_HPP
#define DOT_PARSER_RESOLVER_HPP

#include "non_terminals.hpp"
#include <unordered_set>

// resolve node/edge/graph attributes from dot_graph_raw
// after calling resolve, only possible (non-recursive) statements are node/edge stmts(cuz attrs are resolved)
namespace dot_parser {
    namespace detail {
        // recursive helper of resolve
        dot_graph_resolved resolve_impl(const dot_graph_raw& raw_graph,
                                        external_attrs ext_attrs,
                                        std::unordered_set<std::string>& nodes_seen,
                                        std::unordered_set<edge>& edges_seen);
    }
    dot_graph_resolved resolve(const dot_graph_raw& raw_graph);

    dot_graph_flat flatten(const dot_graph_resolved& resolved_graph);
}

#endif //DOT_PARSER_RESOLVER_HPP
