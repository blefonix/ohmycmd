#include "command_registry.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>

namespace {

int failures = 0;

void expect(bool condition, std::string_view message) {
    if (!condition) {
        ++failures;
        std::cerr << "[FAIL] " << message << '\n';
    }
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
