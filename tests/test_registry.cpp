#include "command_registry.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, std::string_view message) {
    if (!condition) {
        ++failures;
        std::cerr << "[FAIL] " << message << '\n';
    }
}

bool contains(const std::vector<std::string>& values, std::string_view value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

} // namespace

int main() {
    ohmycmd::CommandRegistry registry;

    expect(registry.size() == 0, "registry should start empty");

    expect(registry.registerCommand("Test", "CMD_Test", 7) == ohmycmd::RegisterResult::Ok,
           "register command should succeed");
    expect(registry.registerCommand("test", "CMD_Test2", 0) == ohmycmd::RegisterResult::AlreadyExists,
           "register duplicate should fail");

    expect(registry.addAlias("test", "t") == ohmycmd::AliasResult::Ok, "add alias should succeed");
    expect(registry.addAlias("missing", "x") == ohmycmd::AliasResult::CommandNotFound,
           "add alias to missing command should fail");
    expect(registry.addAlias("test", "t") == ohmycmd::AliasResult::AliasTaken,
           "duplicate alias should fail");

    expect(registry.setDescription("test", "Test description") == ohmycmd::MetadataResult::Ok,
           "set description should succeed");
    expect(registry.setUsage("test", "/test <arg>") == ohmycmd::MetadataResult::Ok,
           "set usage should succeed");
    expect(registry.setFlags("test", 9) == ohmycmd::MetadataResult::Ok,
           "set flags should succeed");
    expect(registry.setCooldown("test", 250, 1000) == ohmycmd::MetadataResult::Ok,
           "set cooldown should succeed");
    expect(registry.setRateLimit("test", 4, 5000) == ohmycmd::MetadataResult::Ok,
           "set rate limit should succeed");

    expect(registry.setCooldown("test", -1, 100) == ohmycmd::MetadataResult::InvalidValue,
           "negative cooldown should fail");
    expect(registry.setRateLimit("test", 10, -10) == ohmycmd::MetadataResult::InvalidValue,
           "negative rate limit window should fail");

    uint32_t flags = 0;
    expect(registry.getFlags("test", flags) == ohmycmd::MetadataResult::Ok, "get flags should succeed");
    expect(flags == 9, "returned flags should match latest value");

    std::string description;
    expect(registry.getDescription("test", description) == ohmycmd::MetadataResult::Ok,
           "get description should succeed");
    expect(description == "Test description", "description getter should match");

    const ohmycmd::CommandSpec* byName = registry.find("test");
    expect(byName != nullptr, "lookup by name should work");
    if (byName != nullptr) {
        expect(byName->name == "test", "normalized name should be lowercase");
        expect(byName->handlerPublic == "CMD_Test", "handler should match");
        expect(byName->flags == 9, "flags should match latest value");
        expect(byName->description == "Test description", "description should match");
        expect(byName->usage == "/test <arg>", "usage should match");
        expect(byName->globalCooldownMs == 250, "global cooldown should match");
        expect(byName->playerCooldownMs == 1000, "player cooldown should match");
        expect(byName->rateLimitCount == 4, "rate limit count should match");
        expect(byName->rateLimitWindowMs == 5000, "rate limit window should match");
    }

    const ohmycmd::CommandSpec* byAlias = registry.find("/t");
    expect(byAlias != nullptr, "lookup by alias should work");
    if (byAlias != nullptr && byName != nullptr) {
        expect(byAlias->name == byName->name, "alias should map to same command");
    }

    expect(registry.exists("test"), "exists should return true for command");
    expect(registry.exists("t"), "exists should return true for alias");
    expect(!registry.exists("missing"), "exists should return false for unknown token");

    auto commands = registry.listCommands();
    expect(commands.size() == 1, "list commands should include one command");
    expect(contains(commands, "test"), "list commands should include canonical name");

    auto aliases = registry.listAliases("test");
    expect(aliases.size() == 1, "list aliases should include alias");
    expect(contains(aliases, "t"), "list aliases should include t");

    expect(registry.renameCommand("t", "tiny") == ohmycmd::RenameResult::Ok,
           "rename alias should succeed");
    expect(registry.exists("tiny"), "renamed alias should exist");
    expect(!registry.exists("t"), "old alias should not exist");

    expect(registry.renameCommand("test", "testing") == ohmycmd::RenameResult::Ok,
           "rename command should succeed");
    expect(registry.exists("testing"), "renamed command should exist");
    expect(registry.exists("tiny"), "alias should survive command rename");
    expect(!registry.exists("test"), "old command name should not exist");

    expect(registry.deleteCommand("tiny") == ohmycmd::DeleteResult::Ok,
           "delete alias should succeed");
    expect(!registry.exists("tiny"), "deleted alias should not exist");

    expect(registry.deleteCommand("testing") == ohmycmd::DeleteResult::Ok,
           "delete command should succeed");
    expect(!registry.exists("testing"), "deleted command should not exist");
    expect(registry.size() == 0, "registry size should be zero after delete command");

    // Case-sensitive mode check.
    registry.setCaseInsensitivity(false);
    expect(registry.registerCommand("Mix", "CMD_Mix", 0) == ohmycmd::RegisterResult::Ok,
           "register mixed-case command in case-sensitive mode should succeed");
    expect(registry.exists("Mix"), "exact-case lookup should work in case-sensitive mode");
    expect(!registry.exists("mix"), "lower-case lookup should fail in case-sensitive mode");

    registry.clear();
    expect(registry.size() == 0, "registry should be empty after clear");
    expect(registry.find("test") == nullptr, "lookup after clear should fail");

    if (failures != 0) {
        std::cerr << "test_registry: " << failures << " failure(s)\n";
        return EXIT_FAILURE;
    }

    std::cout << "test_registry: ok\n";
    return EXIT_SUCCESS;
}
