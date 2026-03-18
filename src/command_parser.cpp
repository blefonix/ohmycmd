#include "command_parser.hpp"

#include <cctype>

namespace ohmycmd {

namespace {

inline bool isSpace(unsigned char c) {
    return std::isspace(c) != 0;
}

std::string_view trimView(std::string_view value) {
    size_t start = 0;
    while (start < value.size() && isSpace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }

    size_t end = value.size();
    while (end > start && isSpace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }

    return value.substr(start, end - start);
}

std::string_view ltrimView(std::string_view value) {
    size_t start = 0;
    while (start < value.size() && isSpace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }
    return value.substr(start);
}

} // namespace

ParsedCommand parseCommandInput(std::string_view input, char prefix) {
    ParsedCommand parsed;

    const std::string_view trimmed = trimView(input);
    if (trimmed.empty()) {
        return parsed;
    }

    parsed.raw.assign(trimmed.begin(), trimmed.end());

    if (trimmed.front() != prefix) {
        return parsed;
    }

    std::string_view body = ltrimView(trimmed.substr(1));
    if (body.empty()) {
        return parsed;
    }

    size_t split = 0;
    while (split < body.size() && !isSpace(static_cast<unsigned char>(body[split]))) {
        ++split;
    }

    std::string_view nameView = body.substr(0, split);
    if (nameView.empty()) {
        return parsed;
    }

    std::string_view argsView;
    if (split < body.size()) {
        argsView = ltrimView(body.substr(split));
    }

    parsed.isCommand = true;
    parsed.name.assign(nameView.begin(), nameView.end());
    parsed.arguments.assign(argsView.begin(), argsView.end());

    return parsed;
}

} // namespace ohmycmd
