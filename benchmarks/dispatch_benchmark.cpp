#include "command_parser.hpp"
#include "command_registry.hpp"

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    int commandCount = 5000;
    int iterations = 500000;

    if (argc >= 2) {
        commandCount = std::max(100, std::atoi(argv[1]));
    }
    if (argc >= 3) {
        iterations = std::max(10000, std::atoi(argv[2]));
    }

    ohmycmd::CommandRegistry registry;

    for (int i = 0; i < commandCount; ++i) {
        const std::string name = "cmd" + std::to_string(i);
        const std::string handler = "CMD_" + name;
        if (registry.registerCommand(name, handler, 0) != ohmycmd::RegisterResult::Ok) {
            std::cerr << "benchmark: register failed i=" << i << '\n';
            return EXIT_FAILURE;
        }

        if (i % 3 == 0) {
            const std::string alias = "c" + std::to_string(i);
            if (registry.addAlias(name, alias) != ohmycmd::AliasResult::Ok) {
                std::cerr << "benchmark: alias failed i=" << i << '\n';
                return EXIT_FAILURE;
            }
        }
    }

    std::mt19937 rng(0xC0FFEEu);
    std::uniform_int_distribution<int> dist(0, commandCount - 1);

    std::vector<std::string> inputs;
    inputs.reserve(static_cast<size_t>(iterations));

    for (int i = 0; i < iterations; ++i) {
        const int idx = dist(rng);
        if ((idx % 3) == 0) {
            inputs.push_back("/c" + std::to_string(idx) + " 100 2.5 note");
        } else {
            inputs.push_back("/cmd" + std::to_string(idx) + " 100 2.5 note");
        }
    }

    // Warm-up
    for (int i = 0; i < 25000 && i < iterations; ++i) {
        const auto parsed = ohmycmd::parseCommandInput(inputs[static_cast<size_t>(i)]);
        if (!parsed.isCommand) {
            std::cerr << "benchmark: parser warm-up failure\n";
            return EXIT_FAILURE;
        }
        (void)registry.find(parsed.name);
    }

    const auto start = std::chrono::steady_clock::now();

    int hits = 0;
    for (int i = 0; i < iterations; ++i) {
        const ohmycmd::ParsedCommand parsed = ohmycmd::parseCommandInput(inputs[static_cast<size_t>(i)]);
        if (!parsed.isCommand) {
            continue;
        }

        if (registry.find(parsed.name) != nullptr) {
            ++hits;
        }
    }

    const auto end = std::chrono::steady_clock::now();
    const auto elapsedNs = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    const double elapsedMs = static_cast<double>(elapsedNs) / 1'000'000.0;
    const double nsPerOp = static_cast<double>(elapsedNs) / static_cast<double>(iterations);
    const double opsPerSec = 1'000'000'000.0 / nsPerOp;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "dispatch_benchmark"
              << " | commands=" << commandCount
              << " | iterations=" << iterations
              << " | hits=" << hits
              << " | elapsed_ms=" << elapsedMs
              << " | ns_per_op=" << nsPerOp
              << " | ops_per_sec=" << opsPerSec
              << '\n';

    return EXIT_SUCCESS;
}
