#ifndef DOT_PARSER_PARSER_HPP
#define DOT_PARSER_PARSER_HPP

#include <string>
#include <utility>
#include <vector>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>

namespace dot_parser {
    struct edge {
        std::string src;
        std::string edge_op;
        std::string tgt;
    };
}

namespace dot_parser {
    namespace dsl = lexy::dsl;
    constexpr auto ws = dsl::whitespace(dsl::ascii::blank);
    constexpr auto wsr = dsl::whitespace(dsl::ascii::blank/dsl::newline);

    // keywords
    constexpr auto keyword_pattern = dsl::identifier(dsl::ascii::alpha);
    struct strict_keyword {
        static constexpr auto rule = dsl::capture(LEXY_LIT("strict"));
        static constexpr auto value = lexy::as_string<std::string>;
    };
    struct graph_keyword {
        static constexpr auto rule = dsl::capture(LEXY_LIT("graph"));
        static constexpr auto value = lexy::as_string<std::string>;
    };
    struct digraph_keyword {
        static constexpr auto rule = dsl::capture(LEXY_LIT("digraph"));
        static constexpr auto value = lexy::as_string<std::string>;
    };
    struct subgraph_keyword {
        static constexpr auto rule = LEXY_KEYWORD("subgraph", keyword_pattern);
    };
    struct node_keyword {
        static constexpr auto rule = LEXY_KEYWORD("node", keyword_pattern);
    };
    struct edge_keyword {
        static constexpr auto rule = LEXY_KEYWORD("edge", keyword_pattern);
    };

    using attr_item_v = std::pair<std::string, std::string>;
    using attr_list_type = std::vector<attr_item_v>;

    struct name {  // either quoted or not quoted
        static constexpr auto escaped_symbols = lexy::symbol_table<char>
                .map<'"'>('"')
                .map<'\\'>('\\')
                .map<'/'>('/')
                .map<'b'>('\b')
                .map<'f'>('\f')
                .map<'n'>('\n')
                .map<'r'>('\r')
                .map<'t'>('\t');
        static constexpr auto rule = [] {
            constexpr auto quoted_r = dsl::ascii::character - dsl::ascii::control;
            auto escape = dsl::backslash_escape.symbol<escaped_symbols>();
            constexpr auto r = dsl::while_one(dsl::ascii::alnum / dsl::lit_c<'_'> / dsl::lit_c<'.'> / dsl::lit_c<'<'> / dsl::lit_c<'>'>);
            return
                    (dsl::peek(dsl::lit_c<'"'>) >> dsl::quoted(quoted_r, escape))
                |   (dsl::else_ >> dsl::capture(r));
        }();
        static constexpr auto value = lexy::as_string<std::string>;
    };
    struct attr_item {
        static constexpr auto rule = dsl::p<name>+ws+dsl::lit_c<'='>+ws+dsl::p<name>;
        static constexpr auto value = lexy::construct<attr_item_v>;
    };
    // attr_stmt
    struct attr_list_non_empty {  // sep by (1) , (2) ; (3) at least one whitespace
        static constexpr auto rule = []{
            constexpr auto bracket = dsl::brackets(dsl::lit_c<'['> >> ws, dsl::lit_c<']'>);
            return bracket.list(dsl::p<attr_item>, dsl::trailing_sep(
                    dsl::peek(dsl::ascii::blank) >> (ws+(dsl::lit_c<';'>/dsl::lit_c<','>/dsl::token(dsl::nullopt))+ws)  // allow blank before ; or ,
                |   (dsl::lit_c<';'> / dsl::lit_c<','>) >> ws
            ));
        }();
        static constexpr auto value = lexy::as_list<attr_list_type>;
    };
    struct attr_list {
        static constexpr auto rule = (dsl::peek(dsl::lit_c<'['>) >> dsl::p<attr_list_non_empty>)
                                   | (dsl::else_ >> dsl::nullopt);
        static constexpr auto value = lexy::callback<attr_list_type>(
                        [](const attr_list_type& v) { return v; },
                        [](lexy::nullopt null_val) { return attr_list_type{}; }
                );
    };
    // three leading keywords: graph, edge, node
    struct attr_stmt_v {
        std::string type;
        attr_list_type attrs;
    };
    struct attr_graph_stmt {
        static constexpr auto rule = dsl::token(graph_keyword::rule) >> (ws+dsl::p<attr_list>);
        static constexpr auto value = lexy::callback<attr_stmt_v>(
            [](const attr_list_type& v) { return attr_stmt_v{"graph", v}; }
        );
    };
    struct attr_edge_stmt {
        static constexpr auto rule = dsl::token(edge_keyword::rule) >> (ws+dsl::p<attr_list>);
        static constexpr auto value = lexy::callback<attr_stmt_v>(
            [](const attr_list_type& v) { return attr_stmt_v{"edge", v}; }
        );
    };
    struct attr_node_stmt {
        static constexpr auto rule = dsl::token(node_keyword::rule) >> (ws+dsl::p<attr_list>);
        static constexpr auto value = lexy::callback<attr_stmt_v>(
            [](const attr_list_type& v) { return attr_stmt_v{"node", v}; }
        );
    };

    struct attr_stmt {
        static constexpr auto rule =[] {
            return dsl::p<attr_graph_stmt> | dsl::p<attr_edge_stmt> | dsl::p<attr_node_stmt>;
        }();
        static constexpr auto value = lexy::forward<attr_stmt_v>;
    };

    struct attr_stmt_vs_item_branch {  // both contain "=", need to tell apart
        static constexpr auto rule = dsl::lookahead(dsl::lit_c<'='>, dsl::lit_c<';'> / dsl::newline) >>
                ((dsl::lookahead(dsl::lit_c<'['>, dsl::lit_c<'='>) >> dsl::p<attr_stmt>)
            |   dsl::else_ >> dsl::p<attr_item>);
        static constexpr auto value = lexy::construct<std::variant<attr_stmt_v, attr_item_v>>;
    };
    // END OF attr_stmt

    // node_stmt; not a branch, needs to be put after else_
    struct node {
        static constexpr auto rule = [] {
            auto lead_char     = dsl::ascii::alpha;
            auto trailing_char = dsl::ascii::alnum / dsl::lit_c<'_'>;
            return dsl::identifier(lead_char, trailing_char);
        }();
        static constexpr auto value = lexy::as_string<std::string>;
    };

    struct node_stmt_v {
        std::string node_name;
        attr_list_type attrs;
    };
    struct node_stmt {
        static constexpr auto rule = dsl::p<node> + ws + dsl::p<attr_list>;
        static constexpr auto value = lexy::construct<node_stmt_v>;
    };
    // END OF node_stmt

    // edge_stmt
    struct node_group {  // {node_1, node_2, node_3}
        static constexpr auto rule = []{
            constexpr auto bracket = dsl::brackets(dsl::lit_c<'{'> >> ws, dsl::lit_c<'}'>);
            return bracket.list(dsl::p<node>, dsl::trailing_sep(
                    dsl::peek(dsl::ascii::blank) >> (ws+(dsl::lit_c<';'>/dsl::lit_c<','>/dsl::token(dsl::nullopt))+ws)  // allow blank before ; or ,
                |   (dsl::lit_c<';'> / dsl::lit_c<','>) >> ws
            ));
        }();
        static constexpr auto value = lexy::as_list<std::vector<std::string>>;
    };

    using nodes_type = std::vector<std::string>;
    struct node_or_node_group {
        static constexpr auto rule = (dsl::peek(dsl::lit_c<'{'>) >> dsl::p<node_group>)
                                   | (dsl::else_ >> dsl::p<node>);
        static constexpr auto value = lexy::callback<nodes_type>(
                    [](const std::string& node) { return nodes_type{node}; },
                    [](const nodes_type& group) { return group; }
                );
    };

    struct edge_op {
        static constexpr auto rule = dsl::capture(LEXY_LIT("--") / LEXY_LIT("->"));
        static constexpr auto value = lexy::as_string<std::string>;
    };

    struct edge_head {  // node_0 ->
        static constexpr auto rule = dsl::p<node_or_node_group>+ws+dsl::p<edge_op>;
        static constexpr auto value = lexy::construct<std::pair<nodes_type, std::string>>;
    };

    struct edge_tail {  // *[node_1 -> node_2]
        static constexpr auto rule = dsl::list(ws+dsl::p<node_or_node_group>+ws, dsl::sep(LEXY_LIT("--") / LEXY_LIT("->")));
        static constexpr auto value = lexy::as_list<std::vector<nodes_type>>;
    };

    struct edge_stmt_v {
        std::vector<edge> edges;
        attr_list_type attrs;
    };

    struct edge_stmt {  // node_1 -> node_2 [type=int]
        static constexpr auto rule = dsl::p<edge_head> + dsl::p<edge_tail> + dsl::p<attr_list>;
        static constexpr auto value = lexy::callback<edge_stmt_v>(
                [](const std::pair<nodes_type, std::string>& head_pair, const std::vector<nodes_type>& tail_vec, const attr_list_type& edge_attrs){
                    std::vector<edge> result;
                    auto src_nodes = head_pair.first;
                    auto& edge_op = head_pair.second;
                    for (auto& tgt_nodes: tail_vec) {
                        for (auto& src: src_nodes) {
                            for (auto& tgt: tgt_nodes) {
                                result.push_back(edge{src, edge_op, tgt});
                            }
                        }
                        src_nodes = tgt_nodes;
                    }
                    return edge_stmt_v{std::move(result), edge_attrs};
                });
    };
    struct edge_stmt_branch {
        static constexpr auto rule = dsl::lookahead(LEXY_LIT("--") / LEXY_LIT("->"), dsl::lit_c<';'> / dsl::newline) >> dsl::p<edge_stmt>;
        static constexpr auto value = lexy::forward<edge_stmt_v>;
    };
    // END OF edge_stmt

    struct stmt_v {
        std::string name;
        std::variant<node_stmt_v, edge_stmt_v, attr_stmt_v, attr_item_v, std::vector<stmt_v>> val;
        stmt_v(const node_stmt_v& v): val{v} {};
        stmt_v(const edge_stmt_v& v): val{v} {};
        stmt_v(const std::variant<attr_stmt_v, attr_item_v>& v) {
            if (std::holds_alternative<attr_stmt_v>(v)) {
                val = std::get<attr_stmt_v>(v);
            } else {
                val = std::get<attr_item_v>(v);
            }
        };
        stmt_v(const std::vector<stmt_v>& v): val{v} {};
        stmt_v(std::string name, const std::vector<stmt_v>& v): name{std::move(name)}, val{v} {}
    };

    struct statement_list {
        static constexpr auto rule = []{
            constexpr auto stmt =
                    dsl::token(subgraph_keyword::rule) >> ws +
                            (dsl::peek(dsl::lit_c<'{'>) >> dsl::recurse<statement_list>  // for unnamed subgraph
                        |   dsl::else_ >> dsl::p<name> + ws + dsl::recurse<statement_list>)  // for named subG
                    | dsl::peek(dsl::lit_c<'{'>) >> ws + dsl::recurse<statement_list>  // for unnamed subgraph
                    | dsl::p<edge_stmt_branch>
                    | dsl::p<attr_stmt_vs_item_branch>
                    | dsl::else_ >> dsl::p<node_stmt>;
            constexpr auto bracket = dsl::brackets(dsl::lit_c<'{'> >> wsr, dsl::lit_c<'}'>);
            return bracket.list(ws+stmt+ws, dsl::trailing_sep((dsl::lit_c<';'>/dsl::newline) >> wsr));
        }();
        static constexpr auto value = lexy::as_list<std::vector<stmt_v>>;
    };

    struct dot_graph_v {
        bool is_strict;
        std::string graph_type;
        std::string name;
        std::vector<stmt_v> statements;
    };

    // top level
    struct dot_graph {
        static constexpr auto rule = []{
            constexpr auto g_keyword = dsl::p<graph_keyword> | dsl::p<digraph_keyword>;
            constexpr auto start = wsr +
                    ((dsl::p<strict_keyword> >> (wsr + g_keyword))
                |   (dsl::else_ >> (dsl::nullopt + g_keyword)))
                + wsr;
            return start + (
                        dsl::peek(dsl::lit_c<'{'>) >> dsl::nullopt + dsl::p<statement_list>
                    |   dsl::else_ >> dsl::p<name> + wsr + dsl::p<statement_list>  // named graph
                    );
        }();
        static constexpr auto value = lexy::callback<dot_graph_v>(
            [](const std::optional<std::string>& strict,
                const std::string& gtype,
                const std::optional<std::string>& gname,
                const std::vector<stmt_v>& stmts) {
                return dot_graph_v{ .is_strict=strict.has_value(),
                                    .graph_type=gtype,
                                    .name=gname.value_or(""),
                                    .statements=stmts };
            }
        );
    };

}

#endif //DOT_PARSER_PARSER_HPP
