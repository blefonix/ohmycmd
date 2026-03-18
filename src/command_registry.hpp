#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ohmycmd {

struct CommandSpec {
    std::string name;
    std::string handlerPublic;
    uint32_t flags = 0;

    std::string description;
    std::string usage;

    int globalCooldownMs = 0;
    int playerCooldownMs = 0;

    int rateLimitCount = 0;
    int rateLimitWindowMs = 0;

    std::vector<std::string> aliases;
};

enum class RegisterResult {
    Ok,
    InvalidName,
    InvalidHandler,
    AlreadyExists
};

enum class AliasResult {
    Ok,
    InvalidAlias,
    CommandNotFound,
    AliasTaken
};

enum class MetadataResult {
    Ok,
    CommandNotFound,
    InvalidValue
};

enum class RenameResult {
    Ok,
    CommandNotFound,
    InvalidName,
    NameTaken
};

enum class DeleteResult {
    Ok,
    CommandNotFound
};

class CommandRegistry {
public:
    RegisterResult registerCommand(std::string_view name, std::string_view handlerPublic, uint32_t flags);
    AliasResult addAlias(std::string_view name, std::string_view alias);

    MetadataResult setFlags(std::string_view name, uint32_t flags);
    MetadataResult getFlags(std::string_view name, uint32_t& flagsOut) const;

    MetadataResult setDescription(std::string_view name, std::string_view text);
    MetadataResult getDescription(std::string_view name, std::string& textOut) const;

    MetadataResult setUsage(std::string_view name, std::string_view text);
    MetadataResult setCooldown(std::string_view name, int globalCooldownMs, int playerCooldownMs);
    MetadataResult setRateLimit(std::string_view name, int maxCalls, int windowMs);

    RenameResult renameCommand(std::string_view name, std::string_view newName);
    DeleteResult deleteCommand(std::string_view name);

    bool exists(std::string_view token) const;
    const CommandSpec* find(std::string_view token) const;

    std::vector<std::string> listCommands() const;
    std::vector<std::string> listAliases(std::string_view name) const;

    void clear();
    size_t size() const;

private:
    using CommandMap = std::unordered_map<std::string, CommandSpec>;
    using AliasMap = std::unordered_map<std::string, std::string>;

    static std::string normalizeIdentifier(std::string_view value);
    static std::string trim(std::string_view value);

    CommandMap::iterator findCommandByToken(std::string_view token);
    CommandMap::const_iterator findCommandByToken(std::string_view token) const;

private:
    CommandMap commandsByName_;
    AliasMap aliasToName_;
};

} // namespace ohmycmd
