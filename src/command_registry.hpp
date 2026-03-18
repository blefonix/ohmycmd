#pragma once

#include <sdk.hpp>

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

class CommandRegistry {
public:
    RegisterResult registerCommand(std::string_view name, std::string_view handlerPublic, uint32_t flags);
    AliasResult addAlias(std::string_view name, std::string_view alias);

    MetadataResult setFlags(std::string_view name, uint32_t flags);
    MetadataResult setDescription(std::string_view name, std::string_view text);
    MetadataResult setUsage(std::string_view name, std::string_view text);
    MetadataResult setCooldown(std::string_view name, int globalCooldownMs, int playerCooldownMs);
    MetadataResult setRateLimit(std::string_view name, int maxCalls, int windowMs);

    const CommandSpec* find(std::string_view token) const;

    void clear();
    size_t size() const;

private:
    static std::string normalizeIdentifier(std::string_view value);
    static std::string trim(std::string_view value);

private:
    std::unordered_map<std::string, CommandSpec> commandsByName_;
    std::unordered_map<std::string, std::string> aliasToName_;
};

} // namespace ohmycmd
