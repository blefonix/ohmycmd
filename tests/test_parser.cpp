#include "command_parser.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

namespace {

int failures = 0;

void expect(bool condition, std::string_view message) {
    if (!condition) {
        ++failures;
        std::cerr << "[FAIL] " << message << '\n';
    }
}

void expectParsed(std::string_view input, bool isCommand, std::string_view name, std::string_view args) {
    const ohmycmd::ParsedCommand parsed = ohmycmd::parseCommandInput(input);

    expect(parsed.isCommand == isCommand, "isCommand mismatch for input='" + std::string(input) + "'");

    if (!isCommand) {
        return;
    }

    expect(parsed.name == name, "name mismatch for input='" + std::string(input) + "'");
    expect(parsed.arguments == args, "arguments mismatch for input='" + std::string(input) + "'");
}

} // namespace

int main() {
    expectParsed("", false, "", "");
    expectParsed("   \t\n", false, "", "");
    expectParsed("hello", false, "", "");
    expectParsed("/", false, "", "");
    expectParsed("/test", true, "test", "");
    expectParsed("/test   ", true, "test", "");
    expectParsed("  /test", true, "test", "");
    expectParsed("/ test", true, "test", "");
    expectParsed("/test a", true, "test", "a");
    expectParsed("/test   alpha beta", true, "test", "alpha beta");
    expectParsed("/TeSt  A1 b2", true, "TeSt", "A1 b2");

    const ohmycmd::ParsedCommand weird = ohmycmd::parseCommandInput(" /test    \t  ");
    expect(weird.isCommand, "weird input should parse as command");
    expect(weird.arguments.empty(), "trailing whitespace should not produce arguments");

    if (failures != 0) {
        std::cerr << "test_parser: " << failures << " failure(s)\n";
        return EXIT_FAILURE;
    }

    std::cout << "test_parser: ok\n";
    return EXIT_SUCCESS;
}
