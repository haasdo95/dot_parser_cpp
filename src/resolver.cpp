#include "resolver.hpp"
#include <cassert>
#include <iostream>

namespace dot_parser {
    namespace detail {
        dot_graph_resolved resolve_impl(const dot_graph_raw& raw_graph,
                                        external_attrs ext_attrs,
                                        std::unordered_set<std::string>& nodes_seen,
                                        std::unordered_set<edge>& edges_seen) {
            dot_graph_resolved resolved { .is_strict=raw_graph.is_strict,
                                          .graph_type=raw_graph.graph_type,  // "graph" or "digraph"
                                          .name=raw_graph.name, .graph_attrs=ext_attrs.graph };

            for (const auto& stmt: raw_graph.statements) {
                if (std::holds_alternative<attr_item_v>(stmt.val)) {  // we decide that the 'ID'='ID' rule adds a private attr to the current graph/subgraph
                    const auto& v = std::get<attr_item_v>(stmt.val);
                    resolved.graph_attrs.insert_or_assign(v.first, v.second);  // no insertion to ext_attrs
                } else if (std::holds_alternative<attr_stmt_v>(stmt.val)) {
                    const auto& v = std::get<attr_stmt_v>(stmt.val);
                    auto& attr_table = [&v, &ext_attrs]() -> auto& {
                        if (v.type=="graph") {
                            return ext_attrs.graph;  // graph[...] adds public attrs, which will be inherited by subgraphs
                        } else if (v.type=="node") {
                            return ext_attrs.node;
                        } else {
                            assert(v.type=="edge");
                            return ext_attrs.edge;
                        }
                    }();
                    for (const auto& attr_pair: v.attrs) {
                        attr_table.insert_or_assign(attr_pair.first, attr_pair.second);  // ensures inheritance
                        if (v.type=="graph") {  // need to alter current graph attr as well
                            resolved.graph_attrs.insert_or_assign(attr_pair.first, attr_pair.second);
                        }
                    }
                } else if (std::holds_alternative<node_stmt_v>(stmt.val)) {
                    const auto& v = std::get<node_stmt_v>(stmt.val);
                    // check node validity
                    if (nodes_seen.contains(v.node_name)) {
                        throw std::runtime_error("redefining node: " + v.node_name);
                    }
                    nodes_seen.insert(v.node_name);
                    // apply external attrs if not already specified by node attrs
                    // gather node attrs first
                    std::map<std::string, std::string> node_attrs {v.attrs.begin(), v.attrs.end()};
                    // insert from ext_attr if not already present
                    node_attrs.insert(ext_attrs.node.begin(), ext_attrs.node.end());
                    // put node attr back
                    std::vector<attr_item_v> new_node_attrs {node_attrs.begin(), node_attrs.end()};
                    resolved.statements.emplace_back(node_stmt_v{ .node_name=v.node_name, .attrs=std::move(new_node_attrs) });
                } else if (std::holds_alternative<edge_stmt_v>(stmt.val)) {
                    const auto& v = std::get<edge_stmt_v>(stmt.val);
                    // check edge validity
                    for (const auto& e: v.edges) {
                        // undefined node(s)
                        if (!nodes_seen.contains(e.src) || !nodes_seen.contains(e.tgt)) {
                            throw std::runtime_error("edge " + e.to_string() + " contains undefined node(s)");
                        }
                        // conflicting edge op
                        if (raw_graph.graph_type=="graph" && e.edge_op=="->") {
                            throw std::runtime_error("directed edge (" + e.to_string() + ") in an undirected graph");
                        } else if (raw_graph.graph_type=="digraph" && e.edge_op=="--") {
                            throw std::runtime_error("undirected edge (" + e.to_string() + ") in a directed graph");
                        }
                        // multi-edge
                        if (raw_graph.is_strict) {  // disallow multi-edge
                            if (edges_seen.contains(e)) {
                                throw std::runtime_error("duplicate edges for a strict graph: " + e.to_string());
                            }
                            edges_seen.insert(e);
                        }
                    }
                    // similar to node stmt
                    std::map<std::string, std::string> edge_attrs {v.attrs.begin(), v.attrs.end()};
                    edge_attrs.insert(ext_attrs.edge.begin(), ext_attrs.edge.end());
                    std::vector<attr_item_v> new_edge_attrs { edge_attrs.begin(), edge_attrs.end() };
                    resolved.statements.emplace_back(edge_stmt_v{ .edges=v.edges, .attrs=std::move(new_edge_attrs) });
                } else {  // a subgraph is encountered
                    assert(std::holds_alternative<std::vector<stmt_v>>(stmt.val));
                    const auto& v = std::get<std::vector<stmt_v>>(stmt.val);
                    // construct a raw graph first
                    dot_graph_raw sub_raw_graph { .is_strict=raw_graph.is_strict,
                                                  .graph_type=raw_graph.graph_type,
                                                  .name=stmt.name,
                                                  .statements=v };
                    // recurse; pass on ext attrs "as is"
                    resolved.statements.emplace_back(resolve_impl(sub_raw_graph, ext_attrs, nodes_seen, edges_seen));
                }
            }
            return resolved;
        }
    }

    dot_graph_resolved resolve(const dot_graph_raw& raw_graph) {
        std::unordered_set<edge> edges_seen;
        std::unordered_set<std::string> nodes_seen;
        detail::external_attrs ext_attrs;
        return detail::resolve_impl(raw_graph, ext_attrs, nodes_seen, edges_seen);
    }

    namespace detail {
        void flatten_impl(const dot_graph_resolved& resolved_graph, std::vector<flat_stmt_type>& statements) {
            std::cerr << "graph attributes of (sub)graph";
            if (!resolved_graph.name.empty()) {
                std::cerr << " with name " << resolved_graph.name;
            }
            std::cerr << " discarded due to flattening\n";
            for (const auto& stmt: resolved_graph.statements) {
                if (std::holds_alternative<detail::node_stmt_v>(stmt)) {
                    const auto& v = std::get<detail::node_stmt_v>(stmt);
                    statements.emplace_back(v);
                } else if (std::holds_alternative<detail::edge_stmt_v>(stmt)) {
                    const auto& v = std::get<detail::edge_stmt_v>(stmt);
                    statements.emplace_back(v);
                } else {
                    assert(std::holds_alternative<dot_graph_resolved>(stmt));
                    const auto& v = std::get<dot_graph_resolved>(stmt);
                    flatten_impl(v, statements);
                }
            }
        }
    }

    dot_graph_flat flatten(const dot_graph_resolved& resolved_graph) {
        dot_graph_flat flat_graph { .is_strict=resolved_graph.is_strict, .graph_type=resolved_graph.graph_type };
        detail::flatten_impl(resolved_graph, flat_graph.statements);
        return flat_graph;
    }
}
