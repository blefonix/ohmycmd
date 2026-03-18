#include "command_parser.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <string_view>

namespace {

std::string trim(std::string_view value) {
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
        ++start;
    }

    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }

    return std::string(value.substr(start, end - start));
}

bool hasWhitespace(std::string_view value) {
    return std::any_of(value.begin(), value.end(), [](unsigned char c) {
        return std::isspace(c) != 0;
    });
}

} // namespace

int main(int argc, char** argv) {
    int iterations = 200000;
    if (argc >= 2) {
        iterations = std::max(1000, std::atoi(argv[1]));
    }

    std::mt19937 rng(0x0BADC0DE);
    std::uniform_int_distribution<int> lenDist(0, 128);
    std::uniform_int_distribution<int> charDist(9, 126);

    for (int i = 0; i < iterations; ++i) {
        const int len = lenDist(rng);

        std::string input;
        input.reserve(static_cast<size_t>(len));

        for (int j = 0; j < len; ++j) {
            char ch = static_cast<char>(charDist(rng));
            input.push_back(ch);
        }

        const ohmycmd::ParsedCommand parsed = ohmycmd::parseCommandInput(input);

        if (!parsed.isCommand) {
            continue;
        }

        if (parsed.name.empty()) {
            std::cerr << "fuzz_parser: parsed empty name at iteration " << i << '\n';
            return EXIT_FAILURE;
        }

        if (hasWhitespace(parsed.name)) {
            std::cerr << "fuzz_parser: parsed command name with whitespace at iteration " << i << '\n';
            return EXIT_FAILURE;
        }

        if (parsed.raw.empty()) {
            std::cerr << "fuzz_parser: parsed command with empty raw at iteration " << i << '\n';
            return EXIT_FAILURE;
        }

        const std::string trimmed = trim(input);
        if (parsed.raw != trimmed) {
            std::cerr << "fuzz_parser: raw mismatch at iteration " << i << '\n';
            return EXIT_FAILURE;
        }

        if (trimmed.empty() || trimmed.front() != '/') {
            std::cerr << "fuzz_parser: parsed command should start with '/' at iteration " << i << '\n';
            return EXIT_FAILURE;
        }
    }

    std::cout << "fuzz_parser: ok (iterations=" << iterations << ")\n";
    return EXIT_SUCCESS;
}
