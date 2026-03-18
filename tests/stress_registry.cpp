#include "command_parser.hpp"
#include "command_registry.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    int commandCount = 10000;
    int lookupCount = 200000;

    if (argc >= 2) {
        commandCount = std::max(1000, std::atoi(argv[1]));
    }
    if (argc >= 3) {
        lookupCount = std::max(10000, std::atoi(argv[2]));
    }

    ohmycmd::CommandRegistry registry;
    std::vector<std::string> names;
    names.reserve(static_cast<size_t>(commandCount));

    for (int i = 0; i < commandCount; ++i) {
        const std::string name = "cmd" + std::to_string(i);
        const std::string handler = "CMD_" + name;

        if (registry.registerCommand(name, handler, static_cast<uint32_t>(i % 8)) != ohmycmd::RegisterResult::Ok) {
            std::cerr << "stress_registry: register failed at i=" << i << '\n';
            return EXIT_FAILURE;
        }

        if (i % 2 == 0) {
            const std::string alias = "c" + std::to_string(i);
            if (registry.addAlias(name, alias) != ohmycmd::AliasResult::Ok) {
                std::cerr << "stress_registry: alias failed at i=" << i << '\n';
                return EXIT_FAILURE;
            }
        }

        if (registry.setCooldown(name, 100 + (i % 50), 400 + (i % 120)) != ohmycmd::MetadataResult::Ok) {
            std::cerr << "stress_registry: cooldown failed at i=" << i << '\n';
            return EXIT_FAILURE;
        }

        if (registry.setRateLimit(name, 3 + (i % 7), 1000 + (i % 500)) != ohmycmd::MetadataResult::Ok) {
            std::cerr << "stress_registry: rate limit failed at i=" << i << '\n';
            return EXIT_FAILURE;
        }

        names.push_back(name);
    }

    if (static_cast<int>(registry.size()) != commandCount) {
        std::cerr << "stress_registry: unexpected size=" << registry.size() << '\n';
        return EXIT_FAILURE;
    }

    std::mt19937 rng(0x51515u);
    std::uniform_int_distribution<int> dist(0, commandCount - 1);

    const auto start = std::chrono::steady_clock::now();

    int hits = 0;
    for (int i = 0; i < lookupCount; ++i) {
        const int idx = dist(rng);

        std::string input;
        if ((idx % 2) == 0) {
            input = "/c" + std::to_string(idx) + " 123";
        } else {
            input = "/" + names[static_cast<size_t>(idx)] + " abc";
        }

        const ohmycmd::ParsedCommand parsed = ohmycmd::parseCommandInput(input);
        if (!parsed.isCommand) {
            std::cerr << "stress_registry: parser failed for input='" << input << "'\n";
            return EXIT_FAILURE;
        }

        const ohmycmd::CommandSpec* spec = registry.find(parsed.name);
        if (spec == nullptr) {
            std::cerr << "stress_registry: lookup failed for token='" << parsed.name << "'\n";
            return EXIT_FAILURE;
        }

        ++hits;
    }

    const auto end = std::chrono::steady_clock::now();
    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "stress_registry: ok"
              << " | commands=" << commandCount
              << " | lookups=" << lookupCount
              << " | hits=" << hits
              << " | elapsed_ms=" << elapsedMs
              << '\n';

    return EXIT_SUCCESS;
}
