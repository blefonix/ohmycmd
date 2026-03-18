#include "command_registry.hpp"

#include <algorithm>
#include <cctype>

namespace ohmycmd {

namespace {

inline bool isSpace(unsigned char c) {
    return std::isspace(c) != 0;
}

} // namespace

std::string CommandRegistry::trim(std::string_view value) {
    size_t start = 0;
    while (start < value.size() && isSpace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }

    size_t end = value.size();
    while (end > start && isSpace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }

    return std::string(value.substr(start, end - start));
}

std::string CommandRegistry::normalizeIdentifier(std::string_view value) {
    std::string out = trim(value);
    if (out.empty()) {
        return out;
    }

    if (out.front() == '/') {
        out.erase(out.begin());
    }

    if (out.empty()) {
        return out;
    }

    for (unsigned char c : out) {
        if (std::isspace(c)) {
            return {};
        }
    }

    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    return out;
}

RegisterResult CommandRegistry::registerCommand(std::string_view name, std::string_view handlerPublic, uint32_t flags) {
    const std::string normalizedName = normalizeIdentifier(name);
    if (normalizedName.empty()) {
        return RegisterResult::InvalidName;
    }

    const std::string normalizedHandler = trim(handlerPublic);
    if (normalizedHandler.empty()) {
        return RegisterResult::InvalidHandler;
    }

    if (commandsByName_.find(normalizedName) != commandsByName_.end()) {
        return RegisterResult::AlreadyExists;
    }

    if (aliasToName_.find(normalizedName) != aliasToName_.end()) {
        return RegisterResult::AlreadyExists;
    }

    CommandSpec spec;
    spec.name = normalizedName;
    spec.handlerPublic = normalizedHandler;
    spec.flags = flags;

    commandsByName_.emplace(normalizedName, std::move(spec));
    aliasToName_.emplace(normalizedName, normalizedName);

    return RegisterResult::Ok;
}

AliasResult CommandRegistry::addAlias(std::string_view name, std::string_view alias) {
    const std::string normalizedName = normalizeIdentifier(name);
    const std::string normalizedAlias = normalizeIdentifier(alias);

    if (normalizedAlias.empty()) {
        return AliasResult::InvalidAlias;
    }

    auto commandIt = commandsByName_.find(normalizedName);
    if (commandIt == commandsByName_.end()) {
        return AliasResult::CommandNotFound;
    }

    if (aliasToName_.find(normalizedAlias) != aliasToName_.end()) {
        return AliasResult::AliasTaken;
    }

    commandIt->second.aliases.push_back(normalizedAlias);
    aliasToName_.emplace(normalizedAlias, normalizedName);
    return AliasResult::Ok;
}

const CommandSpec* CommandRegistry::find(std::string_view token) const {
    const std::string normalized = normalizeIdentifier(token);
    if (normalized.empty()) {
        return nullptr;
    }

    auto aliasIt = aliasToName_.find(normalized);
    if (aliasIt == aliasToName_.end()) {
        return nullptr;
    }

    auto cmdIt = commandsByName_.find(aliasIt->second);
    if (cmdIt == commandsByName_.end()) {
        return nullptr;
    }

    return &cmdIt->second;
}

void CommandRegistry::clear() {
    commandsByName_.clear();
    aliasToName_.clear();
}

size_t CommandRegistry::size() const {
    return commandsByName_.size();
}

} // namespace ohmycmd
