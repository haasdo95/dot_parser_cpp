#include "non_terminals.hpp"
#include <sstream>
#include <cassert>
#include <variant>

namespace dot_parser {
    bool edge::operator==(const edge& other) const {
        if (edge_op==other.edge_op) {
            if (edge_op=="--") {  // undirected
                return (src==other.src && tgt==other.tgt) || (src==other.tgt && tgt==other.src);
            } else {
                assert(edge_op=="->");
                return src==other.src && tgt==other.tgt;
            }
        }
        return false;
    }
    [[nodiscard]] std::size_t edge::hash() const {
        auto compute_ordered = [](const std::string& s1, const std::string& s2){
            std::hash<std::string> str_hasher;
            auto seed = str_hasher(s1);
            // hash_combine from boost
            seed ^= str_hasher(s2) + 0x9e3779b9 + (seed<<6) + (seed>>2);
            return seed;
        };
        if (edge_op=="--") {
            return compute_ordered(src, tgt) + compute_ordered(tgt, src);
        } else {
            assert(edge_op=="->");
            return compute_ordered(src, tgt);
        }
    }
    [[nodiscard]] std::string edge::to_string() const {
        std::stringstream ss;
        ss << src << " " << edge_op << " " << tgt;
        return ss.str();
    }
}

// non-terminal data structures
namespace dot_parser::detail {
    stmt_v::stmt_v(const std::variant<attr_stmt_v, node_stmt_v, attr_item_v>& v)  {
        if (std::holds_alternative<attr_stmt_v>(v)) {
            val = std::get<attr_stmt_v>(v);
        } else if (std::holds_alternative<node_stmt_v>(v)){
            val = std::get<node_stmt_v>(v);
        } else {
            val = std::get<attr_item_v>(v);
        }
    }
}
