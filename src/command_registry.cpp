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

CommandRegistry::CommandMap::iterator CommandRegistry::findCommandByToken(std::string_view token) {
    const std::string normalized = normalizeIdentifier(token);
    if (normalized.empty()) {
        return commandsByName_.end();
    }

    auto aliasIt = aliasToName_.find(normalized);
    if (aliasIt == aliasToName_.end()) {
        return commandsByName_.end();
    }

    return commandsByName_.find(aliasIt->second);
}

CommandRegistry::CommandMap::const_iterator CommandRegistry::findCommandByToken(std::string_view token) const {
    const std::string normalized = normalizeIdentifier(token);
    if (normalized.empty()) {
        return commandsByName_.end();
    }

    auto aliasIt = aliasToName_.find(normalized);
    if (aliasIt == aliasToName_.end()) {
        return commandsByName_.end();
    }

    return commandsByName_.find(aliasIt->second);
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
    const std::string normalizedAlias = normalizeIdentifier(alias);
    if (normalizedAlias.empty()) {
        return AliasResult::InvalidAlias;
    }

    auto commandIt = findCommandByToken(name);
    if (commandIt == commandsByName_.end()) {
        return AliasResult::CommandNotFound;
    }

    if (aliasToName_.find(normalizedAlias) != aliasToName_.end()) {
        return AliasResult::AliasTaken;
    }

    commandIt->second.aliases.push_back(normalizedAlias);
    aliasToName_.emplace(normalizedAlias, commandIt->second.name);
    return AliasResult::Ok;
}

MetadataResult CommandRegistry::setFlags(std::string_view name, uint32_t flags) {
    auto commandIt = findCommandByToken(name);
    if (commandIt == commandsByName_.end()) {
        return MetadataResult::CommandNotFound;
    }

    commandIt->second.flags = flags;
    return MetadataResult::Ok;
}

MetadataResult CommandRegistry::getFlags(std::string_view name, uint32_t& flagsOut) const {
    auto commandIt = findCommandByToken(name);
    if (commandIt == commandsByName_.end()) {
        return MetadataResult::CommandNotFound;
    }

    flagsOut = commandIt->second.flags;
    return MetadataResult::Ok;
}

MetadataResult CommandRegistry::setDescription(std::string_view name, std::string_view text) {
    auto commandIt = findCommandByToken(name);
    if (commandIt == commandsByName_.end()) {
        return MetadataResult::CommandNotFound;
    }

    commandIt->second.description = trim(text);
    return MetadataResult::Ok;
}

MetadataResult CommandRegistry::getDescription(std::string_view name, std::string& textOut) const {
    auto commandIt = findCommandByToken(name);
    if (commandIt == commandsByName_.end()) {
        return MetadataResult::CommandNotFound;
    }

    textOut = commandIt->second.description;
    return MetadataResult::Ok;
}

MetadataResult CommandRegistry::setUsage(std::string_view name, std::string_view text) {
    auto commandIt = findCommandByToken(name);
    if (commandIt == commandsByName_.end()) {
        return MetadataResult::CommandNotFound;
    }

    commandIt->second.usage = trim(text);
    return MetadataResult::Ok;
}

MetadataResult CommandRegistry::setCooldown(std::string_view name, int globalCooldownMs, int playerCooldownMs) {
    if (globalCooldownMs < 0 || playerCooldownMs < 0) {
        return MetadataResult::InvalidValue;
    }

    auto commandIt = findCommandByToken(name);
    if (commandIt == commandsByName_.end()) {
        return MetadataResult::CommandNotFound;
    }

    commandIt->second.globalCooldownMs = globalCooldownMs;
    commandIt->second.playerCooldownMs = playerCooldownMs;
    return MetadataResult::Ok;
}

MetadataResult CommandRegistry::setRateLimit(std::string_view name, int maxCalls, int windowMs) {
    if (maxCalls < 0 || windowMs < 0) {
        return MetadataResult::InvalidValue;
    }

    auto commandIt = findCommandByToken(name);
    if (commandIt == commandsByName_.end()) {
        return MetadataResult::CommandNotFound;
    }

    commandIt->second.rateLimitCount = maxCalls;
    commandIt->second.rateLimitWindowMs = windowMs;
    return MetadataResult::Ok;
}

RenameResult CommandRegistry::renameCommand(std::string_view name, std::string_view newName) {
    const std::string normalizedOld = normalizeIdentifier(name);
    if (normalizedOld.empty()) {
        return RenameResult::CommandNotFound;
    }

    const std::string normalizedNew = normalizeIdentifier(newName);
    if (normalizedNew.empty()) {
        return RenameResult::InvalidName;
    }

    auto oldAliasIt = aliasToName_.find(normalizedOld);
    if (oldAliasIt == aliasToName_.end()) {
        return RenameResult::CommandNotFound;
    }

    if (normalizedNew != normalizedOld && aliasToName_.find(normalizedNew) != aliasToName_.end()) {
        return RenameResult::NameTaken;
    }

    const std::string canonicalName = oldAliasIt->second;
    auto commandIt = commandsByName_.find(canonicalName);
    if (commandIt == commandsByName_.end()) {
        return RenameResult::CommandNotFound;
    }

    // Renaming canonical command name.
    if (normalizedOld == canonicalName) {
        CommandSpec moved = std::move(commandIt->second);
        moved.name = normalizedNew;

        commandsByName_.erase(commandIt);
        auto [newIt, inserted] = commandsByName_.emplace(normalizedNew, std::move(moved));
        if (!inserted) {
            return RenameResult::NameTaken;
        }

        aliasToName_.erase(normalizedOld);
        aliasToName_[normalizedNew] = normalizedNew;

        for (const std::string& alias : newIt->second.aliases) {
            aliasToName_[alias] = normalizedNew;
        }

        return RenameResult::Ok;
    }

    // Renaming alias.
    auto& aliases = commandIt->second.aliases;
    auto aliasVecIt = std::find(aliases.begin(), aliases.end(), normalizedOld);
    if (aliasVecIt == aliases.end()) {
        return RenameResult::CommandNotFound;
    }

    *aliasVecIt = normalizedNew;
    aliasToName_.erase(normalizedOld);
    aliasToName_[normalizedNew] = canonicalName;

    return RenameResult::Ok;
}

DeleteResult CommandRegistry::deleteCommand(std::string_view name) {
    const std::string normalized = normalizeIdentifier(name);
    if (normalized.empty()) {
        return DeleteResult::CommandNotFound;
    }

    auto aliasIt = aliasToName_.find(normalized);
    if (aliasIt == aliasToName_.end()) {
        return DeleteResult::CommandNotFound;
    }

    const std::string canonicalName = aliasIt->second;
    auto commandIt = commandsByName_.find(canonicalName);
    if (commandIt == commandsByName_.end()) {
        return DeleteResult::CommandNotFound;
    }

    // Deleting canonical command removes all aliases.
    if (normalized == canonicalName) {
        for (const std::string& alias : commandIt->second.aliases) {
            aliasToName_.erase(alias);
        }

        aliasToName_.erase(canonicalName);
        commandsByName_.erase(commandIt);
        return DeleteResult::Ok;
    }

    // Deleting alias only.
    auto& aliases = commandIt->second.aliases;
    aliases.erase(std::remove(aliases.begin(), aliases.end(), normalized), aliases.end());
    aliasToName_.erase(normalized);

    return DeleteResult::Ok;
}

bool CommandRegistry::exists(std::string_view token) const {
    return find(token) != nullptr;
}

const CommandSpec* CommandRegistry::find(std::string_view token) const {
    auto cmdIt = findCommandByToken(token);
    if (cmdIt == commandsByName_.end()) {
        return nullptr;
    }

    return &cmdIt->second;
}

std::vector<std::string> CommandRegistry::listCommands() const {
    std::vector<std::string> out;
    out.reserve(commandsByName_.size());

    for (const auto& [name, _] : commandsByName_) {
        (void)_;
        out.push_back(name);
    }

    std::sort(out.begin(), out.end());
    return out;
}

std::vector<std::string> CommandRegistry::listAliases(std::string_view name) const {
    auto commandIt = findCommandByToken(name);
    if (commandIt == commandsByName_.end()) {
        return {};
    }

    std::vector<std::string> out = commandIt->second.aliases;
    std::sort(out.begin(), out.end());
    return out;
}

void CommandRegistry::clear() {
    commandsByName_.clear();
    aliasToName_.clear();
}

size_t CommandRegistry::size() const {
    return commandsByName_.size();
}

} // namespace ohmycmd
