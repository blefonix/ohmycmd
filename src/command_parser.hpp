#pragma once

#include <string>
#include <string_view>

namespace ohmycmd {

struct ParsedCommand {
    bool isCommand = false;
    std::string name;
    std::string arguments;
    std::string raw;
};

ParsedCommand parseCommandInput(std::string_view input, char prefix = '/');

} // namespace ohmycmd
