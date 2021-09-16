#include "parser.hpp"
#include <lexy/input/string_input.hpp>
#include <lexy/input/file.hpp>
#include <lexy/action/parse.hpp> // lexy::parse
#include <lexy_ext/report_error.hpp>

namespace dot_parser {
    dot_graph_raw parse(const std::string& input) {
        auto txt = lexy::string_input(input.c_str(), input.size());
        auto res = lexy::parse<parsing::dot_graph>(txt, lexy_ext::report_error);

        if (res.error_count()) {
            throw std::runtime_error("parsing failed");
        }
        return res.value();
    }

    dot_graph_raw parse_file(const std::string& path) {
        auto file = lexy::read_file(path.c_str());
        if (!file) {
            switch (file.error()) {
                case lexy::file_error::os_error:
                    throw std::runtime_error("error when opening file: " + path);
                    break;
                case lexy::file_error::file_not_found:
                    throw std::runtime_error("file not found: " + path);
                    break;
                case lexy::file_error::permission_denied:
                    throw std::runtime_error("permission denied: " + path);
                    break;
            }
            throw std::runtime_error("unknown error type");
        }

        auto res = lexy::parse<parsing::dot_graph>(file.buffer(), lexy_ext::report_error);
        if (res.error_count()) {
            throw std::runtime_error("parsing failed");
        }
        return res.value();
    }
}
