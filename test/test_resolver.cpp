#include "test_utils.hpp"

TEST(resolver, test_0) {
    std::string inp_small = "graph Students { // no graph prop at all\n"
                      "    node [gender=male, age=21]\n"
                      "    Jack[age=19]  // this overrides the default age=21\n"
                      "    Bob\n"
                      "    node [gender=female, age=21]  // overrides the previous default gender=male\n"
                      "    Alice; Jessica\n"
                      "    edge [status=\"friendzoned\"]\n"
                      "    Alice--Jack; Jack--Bob[status=\"BFF\"]\n"
                      "}";
    auto raw_small = dot_parser::parse(inp_small);
    auto resolved_small = dot_parser::resolve(raw_small);
    std::stringstream ss_small;
    parse_resolved_impl(ss_small, resolved_small, 0);
    std::string sol_small = "Students {\n"
                            "\t[]\n"
                            "\tJack [age=19, gender=male]\n"
                            "\tBob [age=21, gender=male]\n"
                            "\tAlice [age=21, gender=female]\n"
                            "\tJessica [age=21, gender=female]\n"
                            "\tAlice--Jack [status=friendzoned]\n"
                            "\tJack--Bob [status=BFF]\n"
                            "}\n";
    ASSERT_EQ(sol_small, ss_small.str());

    auto raw_graph = dot_parser::parse_file("../test_files/test_0.dot");
    std::string sol_0 = "{\n"
                    "\t[lang=EN, loc=bottom, name=G]\n"
                    "\tA [size=5]\n"
                    "\tB [size=5]\n"
                    "\tC [name=C, size=100]\n"
                    "\tA--B [color=noir]\n"
                    "\tB--C [color=noir]\n"
                    "\tC--A [color=noir]\n"
                    "\tnot_g {\n"
                    "\t\t[lang=CN, loc=top, name=NotG]\n"
                    "\t\tD [size=5]\n"
                    "\t\tE [size=5]\n"
                    "\t\tD--E [color=blanc]\n"
                    "\t\t{\n"
                    "\t\t\t[lang=CN, loc=top, name=G]\n"
                    "\t\t\tE--C [color=noir]\n"
                    "\t\t}\n"
                    "\t}\n"
                    "\t{\n"
                    "\t\t[lang=EN, loc=bottom, name=G]\n"
                    "\t\tF [size=5]\n"
                    "\t\tF--B [color=rouge]\n"
                    "\t\tF--E [color=rouge]\n"
                    "\t}\n"
                    "}\n";
    std::stringstream ss;
    auto resolved = dot_parser::resolve(raw_graph);
    parse_resolved_impl(ss, resolved, 0);
    ASSERT_EQ(ss.str(), sol_0);

    auto flat_g = dot_parser::flatten(resolved);
    ASSERT_EQ(flat_g.statements.size(), 10);
}

TEST(resolver, test_except) {
    std::string dup_node_def_0 = "digraph {A; A;}";
    std::string dup_node_def_1 = "digraph {A; B; subgraph{A;}}";
    std::string undefined_node_0 = "graph {A; A--B}";
    std::string undefined_node_1 = "strict graph {A; subgraph{A--B}}";
    std::string wrong_graph_type_0 = "graph {A; B; A->B}";
    std::string wrong_graph_type_1 = "digraph {A; B; A--B}";
    std::string strictness_0 = "strict graph {A; B; A--B; A--B}";
    std::string strictness_1 = "strict digraph {A; B; {A A}->{B, B}}";
    std::vector<std::string> should_throw = {
            dup_node_def_0, dup_node_def_1,
            undefined_node_0, undefined_node_1,
            wrong_graph_type_0, wrong_graph_type_1,
            strictness_0, strictness_1
    };

    std::string strictness_2 = "graph {A; B; A--B; A--B}";
    std::string strictness_3 = "digraph {A; B; A->{B, B}}";
    std::vector<std::string> should_not_throw = {
            strictness_2, strictness_3
    };

    auto throw_with_msg = [](const std::string& input) {
        auto res = dot_parser::parse(input);
        try {
            dot_parser::resolve(res);
        }
        catch (const std::runtime_error& e) {
            std::cerr << "EXCEPTION: " << e.what() << '\n';
            throw;
        }
    };
    for (const auto& inp: should_throw) {
        ASSERT_ANY_THROW(throw_with_msg(inp));
    }

    for (const auto& inp: should_not_throw) {
        ASSERT_NO_THROW(throw_with_msg(inp));
    }

}