#include "test_utils.hpp"

TEST(test_single, node_stmt) {
    compare("vertex", "vertex\n", parse_node_stmt);
    compare("vertex [ color=red , shape=x;     loc=center size=15; ]", "vertex [color=red, shape=x, loc=center, size=15]\n", parse_node_stmt);
    compare(R"(vertex[color = "no\tir", name = "\"Mike\"", address = "LA\nCalifornia"])",
            "vertex [color=no\tir, name=\"Mike\", address=LA\nCalifornia]\n", parse_node_stmt);
}

TEST(test_single, edge_stmt) {
    compare("A--B", "A--B\n", parse_edge_stmt);
    compare("A -> B", "A->B\n", parse_edge_stmt);
    compare("A -- { B, C } -- D", "A--B\nA--C\nB--D\nC--D\n", parse_edge_stmt);
    compare("A->{B C}->D[size=5]", "A->B [size=5]\nA->C [size=5]\nB->D [size=5]\nC->D [size=5]\n", parse_edge_stmt);
}

TEST(test_single, eq_stmt) {
    compare("size=5", "size=5\n", parse_item_stmt);
    compare(R"(name = "George\nWashington")", "name=George\nWashington\n", parse_item_stmt);
}

TEST(test_single, attr_stmt) {
    compare("edge [size = 2]", "edge [size=2]\n", parse_attr_stmt);
    compare(R"(node[ "color" = "Bl\tanc"; loc=left;])", "node [color=Bl\tanc, loc=left]\n", parse_attr_stmt);
}

TEST(test_many, stmt) {
    std::string solution = "loc=center\n"
                           "node [color=black, alpha=0.75]\n"
                           "{\n"
                           "\tA\n"
                           "\tB\n"
                           "\tC\n"
                           "\tedges {\n"
                           "\t\tA->B\n"
                           "\t\tB->C\n"
                           "\t\tC->A [size=2]\n"
                           "\t}\n"
                           "}\n"
                           "sub_2 {\n"
                           "\tD\n"
                           "\tE\n"
                           "\tD->E\n"
                           "}\n"
                           "X\n"
                           "X->C\n";
    compare("{loc=center; node[color=black; alpha=0.75]\n subgraph {A; B; C; subgraph edges {\nA->B->C\n C->A[size=2]};\n}\n subgraph sub_2{D; E; D->E}\n X; X->C}",
            solution, parse_stmt);
}

TEST(test_many, dot_graph) {
    std::string inp_1 = "strict graph {\n"
                        "\tloc=top\n"
                        "\t{\n"
                        "\t\tA\n"
                        "\t\tB\n"
                        "\t\tA--B\n"
                        "\t}\n"
                        "}\n";
    compare(inp_1, inp_1, parse_dot_graph);
    std::string inp_2 = "\n\tdigraph onion \n{\n graph[size=L]; name=\"layers\"\n{A ; node[color=\"red\"] ; {B\n edge[color=blue]\n {C; {D\n {A->{B, C}->D[size=2]} }} }}}";
    std::string sol_2 = "digraph onion {\n"
                        "\tgraph [size=L]\n"
                        "\tname=layers\n"
                        "\t{\n"
                        "\t\tA\n"
                        "\t\tnode [color=red]\n"
                        "\t\t{\n"
                        "\t\t\tB\n"
                        "\t\t\tedge [color=blue]\n"
                        "\t\t\t{\n"
                        "\t\t\t\tC\n"
                        "\t\t\t\t{\n"
                        "\t\t\t\t\tD\n"
                        "\t\t\t\t\t{\n"
                        "\t\t\t\t\t\tA->B [size=2]\n"
                        "\t\t\t\t\t\tA->C [size=2]\n"
                        "\t\t\t\t\t\tB->D [size=2]\n"
                        "\t\t\t\t\t\tC->D [size=2]\n"
                        "\t\t\t\t\t}\n"
                        "\t\t\t\t}\n"
                        "\t\t\t}\n"
                        "\t\t}\n"
                        "\t}\n"
                        "}\n";
    compare(inp_2, sol_2, parse_dot_graph);
}