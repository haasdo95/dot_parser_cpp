#ifndef DOT_PARSER_PARSER_HPP
#define DOT_PARSER_PARSER_HPP

#include <string>
#include <utility>
#include <vector>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include "non_terminals.hpp"

namespace dot_parser::parsing {
    namespace dsl = lexy::dsl;
    constexpr auto multi_line_comment = dsl::delimited(LEXY_LIT("/*"), LEXY_LIT("*/")).operator()(dsl::ascii::character);
    constexpr auto enclosed_comment = dsl::delimited(LEXY_LIT("/*"), LEXY_LIT("*/")).operator()(dsl::ascii::character-dsl::ascii::newline);
    constexpr auto ws = dsl::whitespace(dsl::ascii::blank | enclosed_comment);  // no line-break in ws; secures look-ahead
    constexpr auto line_comment = LEXY_LIT("//") >> dsl::while_(dsl::ascii::character - dsl::ascii::newline);
    constexpr auto wsr = dsl::whitespace((dsl::ascii::blank/dsl::newline) | multi_line_comment | line_comment);

    // keywords
    constexpr auto keyword_pattern = dsl::identifier(dsl::ascii::alpha);
    constexpr auto graph_keyword = LEXY_KEYWORD("graph", keyword_pattern);
    constexpr auto edge_keyword = LEXY_KEYWORD("edge", keyword_pattern);
    constexpr auto node_keyword = LEXY_KEYWORD("node", keyword_pattern);

    constexpr auto digraph_keyword = LEXY_KEYWORD("digraph", keyword_pattern);

    struct strict_keyword {
        static constexpr auto rule = dsl::capture(LEXY_LIT("strict"));
        static constexpr auto value = lexy::as_string<std::string>;
    };
    constexpr auto subgraph_keyword = LEXY_KEYWORD("subgraph", keyword_pattern);

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
            constexpr auto r = dsl::while_one(dsl::ascii::alpha_digit_underscore
                    / dsl::lit_c<'+'> / dsl::lit_c<'*'>
                    / dsl::lit_c<'.'> / dsl::lit_c<':'> / dsl::lit_c<'!'> / dsl::lit_c<'?'>
                    / dsl::lit_c<'$'> / dsl::lit_c<'%'> / dsl::lit_c<'&'> / dsl::lit_c<'@'>
                    / dsl::lit_c<'('> / dsl::lit_c<')'> / dsl::lit_c<'<'> / dsl::lit_c<'>'>
                    / dsl::lit_c<'\''> / dsl::lit_c<'`'> / dsl::lit_c<'|'> / dsl::lit_c<'^'> / dsl::lit_c<'~'> / dsl::lit_c<'\\'>);
            return
                    (dsl::peek(dsl::lit_c<'"'>) >> dsl::quoted(quoted_r, escape))
                |   (dsl::else_ >> dsl::capture(r));
        }();
        static constexpr auto value = lexy::as_string<std::string>;
    };
    struct attr_item {
        static constexpr auto rule = dsl::p<name>+ws+dsl::lit_c<'='>+ws+dsl::p<name>;
        static constexpr auto value = lexy::construct<detail::attr_item_v>;
    };
    // attr_stmt
    struct attr_list_non_empty {  // sep by (1) , (2) ; (3) at least one whitespace
        static constexpr auto rule = []{
            constexpr auto bracket = dsl::brackets(dsl::lit_c<'['> >> ws, dsl::lit_c<']'>);
            return bracket.list(dsl::p<attr_item>, dsl::trailing_sep(
                    // allow blank or in-line comments before ; or ,
                    dsl::peek(dsl::ascii::blank / LEXY_LIT("/*")) >> (ws+(dsl::lit_c<';'>/dsl::lit_c<','>/dsl::token(dsl::nullopt))+ws)
                |   (dsl::lit_c<';'> / dsl::lit_c<','>) >> ws
            ));
        }();
        static constexpr auto value = lexy::as_list<detail::attr_list_type>;
    };
    struct attr_list {
        static constexpr auto rule = (dsl::peek(dsl::lit_c<'['>) >> dsl::p<attr_list_non_empty>)
                                   | (dsl::else_ >> dsl::nullopt);
        static constexpr auto value = lexy::callback<detail::attr_list_type>(
                        [](const detail::attr_list_type& v) { return v; },
                        [](lexy::nullopt null_val) { return detail::attr_list_type{}; }
                );
    };
    // three leading keywords: graph, edge, node
    struct attr_graph_stmt {
        static constexpr auto rule = graph_keyword >> (ws+dsl::p<attr_list>);
        static constexpr auto value = lexy::callback<detail::attr_stmt_v>(
            [](const detail::attr_list_type& v) { return detail::attr_stmt_v{"graph", v}; }
        );
    };
    struct attr_edge_stmt {
        static constexpr auto rule = edge_keyword >> (ws+dsl::p<attr_list>);
        static constexpr auto value = lexy::callback<detail::attr_stmt_v>(
            [](const detail::attr_list_type& v) { return detail::attr_stmt_v{"edge", v}; }
        );
    };
    struct attr_node_stmt {
        static constexpr auto rule = node_keyword >> (ws+dsl::p<attr_list>);
        static constexpr auto value = lexy::callback<detail::attr_stmt_v>(
            [](const detail::attr_list_type& v) { return detail::attr_stmt_v{"node", v}; }
        );
    };

    struct attr_stmt {
        static constexpr auto rule = dsl::p<attr_graph_stmt> | dsl::p<attr_edge_stmt> | dsl::p<attr_node_stmt>;
        static constexpr auto value = lexy::forward<detail::attr_stmt_v>;
    };
    // END OF attr_stmt

    // node_stmt; not a branch, needs to be put after else_
    struct node_stmt {
        static constexpr auto rule = dsl::p<name> + ws + dsl::p<attr_list>;
        static constexpr auto value = lexy::construct<detail::node_stmt_v>;
    };
    // END OF node_stmt

    struct attr_stmt_vs_item_vs_node_branch {  // all contain "=", need to tell apart
        static constexpr auto rule = dsl::lookahead(dsl::lit_c<'='>, dsl::lit_c<';'> / dsl::newline) >>
                ((dsl::lookahead(dsl::lit_c<'['>, dsl::lit_c<'='>) >>
                        ((dsl::peek(graph_keyword/edge_keyword/node_keyword) >> dsl::p<attr_stmt>)
                    |   dsl::else_ >> dsl::p<node_stmt>))
                |   dsl::else_ >> dsl::p<attr_item>);
        static constexpr auto value = lexy::construct<std::variant<detail::attr_stmt_v, detail::node_stmt_v, detail::attr_item_v>>;
    };

    // edge_stmt
    struct node_group {  // {node_1, node_2, node_3}
        static constexpr auto rule = []{
            constexpr auto bracket = dsl::brackets(dsl::lit_c<'{'> >> ws, dsl::lit_c<'}'>);
            return bracket.list(dsl::p<name>, dsl::trailing_sep(
                    // allow blank or in-line comments before ; or ,
                    dsl::peek(dsl::ascii::blank / LEXY_LIT("/*")) >> (ws+(dsl::lit_c<','>/dsl::token(dsl::nullopt))+ws)
                |   (dsl::lit_c<','>) >> ws
            ));
        }();
        static constexpr auto value = lexy::as_list<std::vector<std::string>>;
    };

    using nodes_type = std::vector<std::string>;
    struct node_or_node_group {
        static constexpr auto rule = (dsl::peek(dsl::lit_c<'{'>) >> dsl::p<node_group>)
                                   | (dsl::else_ >> dsl::p<name>);
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

    struct edge_stmt {  // node_1 -> node_2 [type=int]
        static constexpr auto rule = dsl::p<edge_head> + dsl::p<edge_tail> + dsl::p<attr_list>;
        static constexpr auto value = lexy::callback<detail::edge_stmt_v>(
                [](const std::pair<nodes_type, std::string>& head_pair, const std::vector<nodes_type>& tail_vec, const detail::attr_list_type& edge_attrs){
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
                    return detail::edge_stmt_v{std::move(result), edge_attrs};
                });
    };
    struct edge_stmt_branch {
        static constexpr auto rule = dsl::lookahead(LEXY_LIT("--") / LEXY_LIT("->"), dsl::lit_c<';'> / dsl::newline) >> dsl::p<edge_stmt>;
        static constexpr auto value = lexy::forward<detail::edge_stmt_v>;
    };
    // END OF edge_stmt

    struct statement_list {
        static constexpr auto rule = []{
            constexpr auto stmt =
                    subgraph_keyword >> wsr +
                            (dsl::peek(dsl::lit_c<'{'>) >> dsl::recurse<statement_list>  // for unnamed subgraph: subgraph {...}
                        |   dsl::else_ >> dsl::p<name> + wsr + dsl::recurse<statement_list>)  // for named sub: subgraph sub_g {...}
                    | dsl::p<edge_stmt_branch>  // edge stmt may also start by '{', which is why we must disallow subgraph without keyword
                    | dsl::p<attr_stmt_vs_item_vs_node_branch>
                    | line_comment
                    | dsl::else_ >> dsl::p<node_stmt>;
            constexpr auto bracket = dsl::brackets(dsl::lit_c<'{'> >> wsr, dsl::lit_c<'}'>);
            return bracket.list(ws+stmt+ws, dsl::trailing_sep(
                        ((dsl::lit_c<';'>/dsl::newline)
                    |   line_comment) >> wsr
                   ));
        }();
        static constexpr auto value = lexy::as_list<std::vector<detail::stmt_v>>;
    };

    struct g_keyword {
        static constexpr auto rule = dsl::capture(graph_keyword/digraph_keyword);
        static constexpr auto value = lexy::as_string<std::string>;
    };

    // top level
    struct dot_graph {
        static constexpr auto rule = []{
            constexpr auto start = wsr +
                    ((dsl::p<strict_keyword> >> (wsr + dsl::p<g_keyword>))
                |   (dsl::else_ >> (dsl::nullopt + dsl::p<g_keyword>)))
                + wsr;
            return start + (
                        dsl::peek(dsl::lit_c<'{'>) >> dsl::nullopt + wsr + dsl::p<statement_list>
                    |   dsl::else_ >> dsl::p<name> + wsr + dsl::p<statement_list>  // named graph
                    );
        }();
        static constexpr auto value = lexy::callback<dot_graph_raw>(
            [](const std::optional<std::string>& strict,
                const std::string & gtype,
                const std::optional<std::string>& gname,
                const std::vector<detail::stmt_v>& stmts) {
                return dot_graph_raw{ .is_strict=strict.has_value(),
                                    .graph_type=gtype,
                                    .name=gname.value_or(""),
                                    .statements=stmts };
            }
        );
    };

}

// parsing API
namespace dot_parser {
    dot_graph_raw parse(const std::string& input);
    dot_graph_raw parse_file(const std::string& path);
}

#endif //DOT_PARSER_PARSER_HPP
