#include <sdk.hpp>
#include <Server/Components/Pawn/pawn.hpp>

#include "command_parser.hpp"
#include "command_registry.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cerrno>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

std::string trim(std::string_view value) {
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
        ++start;
    }

    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }

    return std::string(value.substr(start, end - start));
}

std::string toLowerAscii(std::string_view value) {
    std::string out(value.begin(), value.end());
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return out;
}

std::vector<std::string> tokenizeArgs(std::string_view input) {
    std::vector<std::string> tokens;
    std::string current;

    bool inQuotes = false;

    for (char ch : input) {
        if (ch == '"') {
            inQuotes = !inQuotes;
            continue;
        }

        if (!inQuotes && std::isspace(static_cast<unsigned char>(ch)) != 0) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }

        current.push_back(ch);
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

bool parseIntStrict(std::string_view value, int& out) {
    if (value.empty()) {
        return false;
    }

    const char* begin = value.data();
    const char* end = value.data() + value.size();
    const auto [ptr, ec] = std::from_chars(begin, end, out);
    return ec == std::errc() && ptr == end;
}

bool parseFloatStrict(std::string_view value, float& out) {
    if (value.empty()) {
        return false;
    }

    std::string tmp(value.begin(), value.end());
    char* endPtr = nullptr;

    errno = 0;
    const float parsed = std::strtof(tmp.c_str(), &endPtr);
    if (endPtr == nullptr || *endPtr != '\0' || errno == ERANGE) {
        return false;
    }

    out = parsed;
    return true;
}

int remainingMs(TimePoint now, TimePoint until) {
    if (until <= now) {
        return 0;
    }

    const auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(until - now).count();
    if (delta <= 0) {
        return 0;
    }

    if (delta > std::numeric_limits<int>::max()) {
        return std::numeric_limits<int>::max();
    }

    return static_cast<int>(delta);
}

class OhMyCmdComponent;
OhMyCmdComponent* g_instance = nullptr;

class OhMyCmdComponent final : public IComponent, public PlayerTextEventHandler, public PawnEventHandler {
    PROVIDE_UID(0x8c97f1b3d67a4f29);

public:
    StringView componentName() const override {
        return "ohmycmd";
    }

    SemanticVersion componentVersion() const override {
        return SemanticVersion(OHMYCMD_VERSION_MAJOR, OHMYCMD_VERSION_MINOR, OHMYCMD_VERSION_PATCH, 0);
    }

    void onLoad(ICore* core) override {
        core_ = core;
        g_instance = this;

        if (core_ != nullptr) {
            core_->printLn("[ohmycmd] onLoad: component loaded");
        }
    }

    void onInit(IComponentList* components) override {
        components_ = components;
        pawnComponent_ = (components_ != nullptr) ? components_->queryComponent<IPawnComponent>() : nullptr;

        if (pawnComponent_ != nullptr) {
            pawnHooked_ = pawnComponent_->getEventDispatcher().addEventHandler(this);
            if (core_ != nullptr) {
                core_->printLn("[ohmycmd] onInit: pawn component detected (%s)", pawnHooked_ ? "hooked" : "hook-failed");
            }
        } else if (core_ != nullptr) {
            core_->logLn(LogLevel::Warning, "[ohmycmd] onInit: pawn component not found");
        }
    }

    void onReady() override {
        if (core_ != nullptr) {
            playerTextHooked_ = core_->getPlayers().getPlayerTextDispatcher().addEventHandler(this, EventPriority_FairlyHigh);
            core_->printLn("[ohmycmd] onReady: runtime core active (%s)", playerTextHooked_ ? "player-hooked" : "player-hook-failed");
        }
    }

    void onFree(IComponent* component) override {
        if (component == pawnComponent_) {
            pawnComponent_ = nullptr;
            pawnHooked_ = false;
            if (core_ != nullptr) {
                core_->logLn(LogLevel::Debug, "[ohmycmd] onFree: pawn component released");
            }
        }
    }

    void reset() override {
        registry_.clear();
        globalCooldownUntil_.clear();
        playerCooldownUntil_.clear();
        rateBuckets_.clear();

        if (core_ != nullptr) {
            core_->printLn("[ohmycmd] reset: registry and policy state cleared");
        }
    }

    void free() override {
        detachEventHandlers();

        if (core_ != nullptr) {
            core_->printLn("[ohmycmd] free");
        }

        g_instance = nullptr;
        delete this;
    }

    // PlayerTextEventHandler
    bool onPlayerCommandText(IPlayer& player, StringView message) override {
        const ohmycmd::ParsedCommand parsed = ohmycmd::parseCommandInput(std::string_view(message.data(), message.length()));
        if (!parsed.isCommand) {
            return false;
        }

        const ohmycmd::CommandSpec* command = registry_.find(parsed.name);
        if (command == nullptr) {
            return false;
        }

        if (isHelpRequest(parsed.arguments)) {
            sendCommandHelp(player, *command);
            return true;
        }

        IPawnScript* script = mainScript();

        const TimePoint now = Clock::now();
        const PolicyDecision policy = evaluatePolicy(script, player, *command, now);
        if (!policy.allowed) {
            handlePolicyDenied(script, player, *command, policy.reason, policy.retryMs);
            return true;
        }

        consumeRateLimit(player, *command, now);

        const DispatchResult dispatch = invokePawnHandler(player, *command, parsed.arguments);
        if (dispatch == DispatchResult::Failed) {
            player.sendClientMessage(Colour(255, 80, 80), "[ohmycmd] handler failed. Check server logs.");
            return true;
        }

        applyCooldown(player, *command, now);

        if (dispatch == DispatchResult::CalledNotHandled) {
            sendCommandUsage(player, *command);
        }

        return true;
    }

    // PawnEventHandler
    void onAmxLoad(IPawnScript& script) override {
        static const std::array<AMX_NATIVE_INFO, 16> kNatives = {{
            { "OMC_Register", &OhMyCmdComponent::Native_OhmyCmd_Register },
            { "OMC_RegisterCompat", &OhMyCmdComponent::Native_OhmyCmd_RegisterCompat },
            { "OMC_AddAlias", &OhMyCmdComponent::Native_OhmyCmd_AddAlias },
            { "OMC_SetFlags", &OhMyCmdComponent::Native_OhmyCmd_SetFlags },
            { "OMC_SetDescription", &OhMyCmdComponent::Native_OhmyCmd_SetDescription },
            { "OMC_SetUsage", &OhMyCmdComponent::Native_OhmyCmd_SetUsage },
            { "OMC_SetCooldown", &OhMyCmdComponent::Native_OhmyCmd_SetCooldown },
            { "OMC_SetRateLimit", &OhMyCmdComponent::Native_OhmyCmd_SetRateLimit },
            { "OMC_Execute", &OhMyCmdComponent::Native_OhmyCmd_Execute },
            { "OMC_Count", &OhMyCmdComponent::Native_OhmyCmd_Count },
            { "OMC_ArgCount", &OhMyCmdComponent::Native_OhmyCmd_ArgCount },
            { "OMC_ArgInt", &OhMyCmdComponent::Native_OhmyCmd_ArgInt },
            { "OMC_ArgFloat", &OhMyCmdComponent::Native_OhmyCmd_ArgFloat },
            { "OMC_ArgPlayerID", &OhMyCmdComponent::Native_OhmyCmd_ArgPlayerID },
            { "OMC_ArgString", &OhMyCmdComponent::Native_OhmyCmd_ArgString },
            { "OMC_ArgRest", &OhMyCmdComponent::Native_OhmyCmd_ArgRest },
        }};

        const int err = script.Register(kNatives.data(), static_cast<int>(kNatives.size()));
        if (core_ != nullptr) {
            if (err == AMX_ERR_NONE) {
                core_->logLn(LogLevel::Debug, "[ohmycmd] natives registered for script id=%d", script.GetID());
            } else if (err == AMX_ERR_NOTFOUND || err == AMX_ERR_INDEX) {
                core_->logLn(LogLevel::Debug,
                             "[ohmycmd] no ohmycmd natives referenced in script id=%d (err=%d)",
                             script.GetID(),
                             err);
            } else {
                core_->logLn(LogLevel::Error, "[ohmycmd] native registration failed for script id=%d err=%d", script.GetID(), err);
            }
        }
    }

    void onAmxUnload(IPawnScript& script) override {
        if (pawnComponent_ != nullptr && pawnComponent_->mainScript() == &script) {
            registry_.clear();
            globalCooldownUntil_.clear();
            playerCooldownUntil_.clear();
            rateBuckets_.clear();

            if (core_ != nullptr) {
                core_->logLn(LogLevel::Debug, "[ohmycmd] main script unloaded: registry/policy state cleared");
            }
        }
    }

private:
    enum class DispatchResult {
        Failed,
        CalledHandled,
        CalledNotHandled
    };

    enum class DenyReason : int {
        Permission = 1,
        GlobalCooldown = 2,
        PlayerCooldown = 3,
        RateLimit = 4
    };

    struct PolicyDecision {
        bool allowed = true;
        DenyReason reason = DenyReason::Permission;
        int retryMs = 0;
    };

    struct RateBucket {
        TimePoint windowStart {};
        int calls = 0;
    };

    static cell Native_OhmyCmd_Register(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeRegister(amx, params) : 0;
    }

    static cell Native_OhmyCmd_RegisterCompat(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeRegisterCompat(amx, params) : 0;
    }

    static cell Native_OhmyCmd_AddAlias(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeAddAlias(amx, params) : 0;
    }

    static cell Native_OhmyCmd_SetFlags(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeSetFlags(amx, params) : 0;
    }

    static cell Native_OhmyCmd_SetDescription(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeSetDescription(amx, params) : 0;
    }

    static cell Native_OhmyCmd_SetUsage(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeSetUsage(amx, params) : 0;
    }

    static cell Native_OhmyCmd_SetCooldown(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeSetCooldown(amx, params) : 0;
    }

    static cell Native_OhmyCmd_SetRateLimit(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeSetRateLimit(amx, params) : 0;
    }

    static cell Native_OhmyCmd_Execute(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeExecute(amx, params) : 0;
    }

    static cell Native_OhmyCmd_Count(AMX* amx, cell* params) {
        (void)amx;
        (void)params;
        return g_instance != nullptr ? static_cast<cell>(g_instance->registry_.size()) : 0;
    }

    static cell Native_OhmyCmd_ArgCount(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeArgCount(amx, params) : 0;
    }

    static cell Native_OhmyCmd_ArgInt(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeArgInt(amx, params) : 0;
    }

    static cell Native_OhmyCmd_ArgFloat(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeArgFloat(amx, params) : 0;
    }

    static cell Native_OhmyCmd_ArgPlayerID(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeArgPlayerID(amx, params) : 0;
    }

    static cell Native_OhmyCmd_ArgString(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeArgString(amx, params) : 0;
    }

    static cell Native_OhmyCmd_ArgRest(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeArgRest(amx, params) : 0;
    }

    cell nativeRegister(AMX* amx, cell* params) {
        const int argc = params[0] / static_cast<int>(sizeof(cell));
        if (argc < 2) {
            return 0;
        }

        const std::string name = readAmxString(amx, params[1]);
        const std::string handler = readAmxString(amx, params[2]);
        const uint32_t flags = argc >= 3 ? static_cast<uint32_t>(params[3]) : 0U;

        const ohmycmd::RegisterResult result = registry_.registerCommand(name, handler, flags);
        if (core_ != nullptr) {
            switch (result) {
                case ohmycmd::RegisterResult::Ok:
                    core_->logLn(LogLevel::Debug, "[ohmycmd] register: /%s -> %s (flags=%u)", name.c_str(), handler.c_str(), flags);
                    break;
                case ohmycmd::RegisterResult::InvalidName:
                    core_->logLn(LogLevel::Warning, "[ohmycmd] register failed: invalid command name");
                    break;
                case ohmycmd::RegisterResult::InvalidHandler:
                    core_->logLn(LogLevel::Warning, "[ohmycmd] register failed: invalid handler for /%s", name.c_str());
                    break;
                case ohmycmd::RegisterResult::AlreadyExists:
                    core_->logLn(LogLevel::Warning, "[ohmycmd] register failed: command already exists: /%s", name.c_str());
                    break;
            }
        }

        return result == ohmycmd::RegisterResult::Ok ? 1 : 0;
    }

    cell nativeRegisterCompat(AMX* amx, cell* params) {
        const int argc = params[0] / static_cast<int>(sizeof(cell));
        if (argc < 1) {
            return 0;
        }

        const std::string inputName = readAmxString(amx, params[1]);
        const uint32_t flags = argc >= 2 ? static_cast<uint32_t>(params[2]) : 0U;

        IPawnScript* script = mainScript();
        if (script == nullptr) {
            if (core_ != nullptr) {
                core_->logLn(LogLevel::Warning, "[ohmycmd] register-compat failed: main script unavailable");
            }
            return 0;
        }

        std::string normalized = trim(inputName);
        if (!normalized.empty() && normalized.front() == '/') {
            normalized.erase(normalized.begin());
        }
        if (normalized.empty() || normalized.find_first_of(" \t\r\n") != std::string::npos) {
            if (core_ != nullptr) {
                core_->logLn(LogLevel::Warning, "[ohmycmd] register-compat failed: invalid command name '%s'", inputName.c_str());
            }
            return 0;
        }

        const std::string lower = toLowerAscii(normalized);
        std::vector<std::string> candidates;
        candidates.reserve(8);

        auto pushCandidate = [&candidates](std::string value) {
            if (value.empty()) {
                return;
            }

            if (std::find(candidates.begin(), candidates.end(), value) == candidates.end()) {
                candidates.push_back(std::move(value));
            }
        };

        pushCandidate("OMC_" + normalized);
        pushCandidate("CMD_" + normalized);
        pushCandidate("cmd_" + normalized);
        pushCandidate(normalized);

        if (lower != normalized) {
            pushCandidate("OMC_" + lower);
            pushCandidate("CMD_" + lower);
            pushCandidate("cmd_" + lower);
            pushCandidate(lower);
        }

        std::string resolvedHandler;
        for (const std::string& candidate : candidates) {
            int publicIndex = std::numeric_limits<int>::max();
            if (script->FindPublic(candidate.c_str(), &publicIndex) == AMX_ERR_NONE && publicIndex != std::numeric_limits<int>::max()) {
                resolvedHandler = candidate;
                break;
            }
        }

        if (resolvedHandler.empty()) {
            if (core_ != nullptr) {
                core_->logLn(LogLevel::Warning,
                             "[ohmycmd] register-compat failed: no compatible public found for /%s (tried %d patterns)",
                             normalized.c_str(),
                             static_cast<int>(candidates.size()));
            }
            return 0;
        }

        const ohmycmd::RegisterResult result = registry_.registerCommand(normalized, resolvedHandler, flags);
        if (core_ != nullptr) {
            switch (result) {
                case ohmycmd::RegisterResult::Ok:
                    core_->logLn(LogLevel::Debug,
                                 "[ohmycmd] register-compat: /%s -> %s (flags=%u)",
                                 normalized.c_str(),
                                 resolvedHandler.c_str(),
                                 flags);
                    break;
                case ohmycmd::RegisterResult::InvalidName:
                    core_->logLn(LogLevel::Warning, "[ohmycmd] register-compat failed: invalid command name /%s", normalized.c_str());
                    break;
                case ohmycmd::RegisterResult::InvalidHandler:
                    core_->logLn(LogLevel::Warning, "[ohmycmd] register-compat failed: invalid handler for /%s", normalized.c_str());
                    break;
                case ohmycmd::RegisterResult::AlreadyExists:
                    core_->logLn(LogLevel::Warning, "[ohmycmd] register-compat failed: command already exists /%s", normalized.c_str());
                    break;
            }
        }

        return result == ohmycmd::RegisterResult::Ok ? 1 : 0;
    }

    cell nativeAddAlias(AMX* amx, cell* params) {
        const int argc = params[0] / static_cast<int>(sizeof(cell));
        if (argc < 2) {
            return 0;
        }

        const std::string name = readAmxString(amx, params[1]);
        const std::string alias = readAmxString(amx, params[2]);

        const ohmycmd::AliasResult result = registry_.addAlias(name, alias);
        if (core_ != nullptr) {
            switch (result) {
                case ohmycmd::AliasResult::Ok:
                    core_->logLn(LogLevel::Debug, "[ohmycmd] alias: /%s -> /%s", alias.c_str(), name.c_str());
                    break;
                case ohmycmd::AliasResult::InvalidAlias:
                    core_->logLn(LogLevel::Warning, "[ohmycmd] alias failed: invalid alias for /%s", name.c_str());
                    break;
                case ohmycmd::AliasResult::CommandNotFound:
                    core_->logLn(LogLevel::Warning, "[ohmycmd] alias failed: command not found /%s", name.c_str());
                    break;
                case ohmycmd::AliasResult::AliasTaken:
                    core_->logLn(LogLevel::Warning, "[ohmycmd] alias failed: already used /%s", alias.c_str());
                    break;
            }
        }

        return result == ohmycmd::AliasResult::Ok ? 1 : 0;
    }

    cell nativeSetFlags(AMX* amx, cell* params) {
        const int argc = params[0] / static_cast<int>(sizeof(cell));
        if (argc < 2) {
            return 0;
        }

        const std::string name = readAmxString(amx, params[1]);
        const uint32_t flags = static_cast<uint32_t>(params[2]);

        const ohmycmd::MetadataResult result = registry_.setFlags(name, flags);
        if (core_ != nullptr) {
            switch (result) {
                case ohmycmd::MetadataResult::Ok:
                    core_->logLn(LogLevel::Debug, "[ohmycmd] flags set: /%s flags=%u", name.c_str(), flags);
                    break;
                case ohmycmd::MetadataResult::CommandNotFound:
                    core_->logLn(LogLevel::Warning, "[ohmycmd] flags set failed: command not found /%s", name.c_str());
                    break;
                case ohmycmd::MetadataResult::InvalidValue:
                    core_->logLn(LogLevel::Warning, "[ohmycmd] flags set failed: invalid value /%s", name.c_str());
                    break;
            }
        }

        return result == ohmycmd::MetadataResult::Ok ? 1 : 0;
    }

    cell nativeSetDescription(AMX* amx, cell* params) {
        const int argc = params[0] / static_cast<int>(sizeof(cell));
        if (argc < 2) {
            return 0;
        }

        const std::string name = readAmxString(amx, params[1]);
        const std::string text = readAmxString(amx, params[2]);

        const ohmycmd::MetadataResult result = registry_.setDescription(name, text);
        if (core_ != nullptr) {
            switch (result) {
                case ohmycmd::MetadataResult::Ok:
                    core_->logLn(LogLevel::Debug, "[ohmycmd] description set: /%s", name.c_str());
                    break;
                case ohmycmd::MetadataResult::CommandNotFound:
                    core_->logLn(LogLevel::Warning, "[ohmycmd] description failed: command not found /%s", name.c_str());
                    break;
                case ohmycmd::MetadataResult::InvalidValue:
                    core_->logLn(LogLevel::Warning, "[ohmycmd] description failed: invalid value /%s", name.c_str());
                    break;
            }
        }

        return result == ohmycmd::MetadataResult::Ok ? 1 : 0;
    }

    cell nativeSetUsage(AMX* amx, cell* params) {
        const int argc = params[0] / static_cast<int>(sizeof(cell));
        if (argc < 2) {
            return 0;
        }

        const std::string name = readAmxString(amx, params[1]);
        const std::string text = readAmxString(amx, params[2]);

        const ohmycmd::MetadataResult result = registry_.setUsage(name, text);
        if (core_ != nullptr) {
            switch (result) {
                case ohmycmd::MetadataResult::Ok:
                    core_->logLn(LogLevel::Debug, "[ohmycmd] usage set: /%s", name.c_str());
                    break;
                case ohmycmd::MetadataResult::CommandNotFound:
                    core_->logLn(LogLevel::Warning, "[ohmycmd] usage failed: command not found /%s", name.c_str());
                    break;
                case ohmycmd::MetadataResult::InvalidValue:
                    core_->logLn(LogLevel::Warning, "[ohmycmd] usage failed: invalid value /%s", name.c_str());
                    break;
            }
        }

        return result == ohmycmd::MetadataResult::Ok ? 1 : 0;
    }

    cell nativeSetCooldown(AMX* amx, cell* params) {
        const int argc = params[0] / static_cast<int>(sizeof(cell));
        if (argc < 1) {
            return 0;
        }

        const std::string name = readAmxString(amx, params[1]);
        const int globalMs = argc >= 2 ? static_cast<int>(params[2]) : 0;
        const int playerMs = argc >= 3 ? static_cast<int>(params[3]) : 0;

        const ohmycmd::MetadataResult result = registry_.setCooldown(name, globalMs, playerMs);
        if (result == ohmycmd::MetadataResult::Ok) {
            if (const ohmycmd::CommandSpec* command = registry_.find(name); command != nullptr) {
                globalCooldownUntil_.erase(command->name);
                playerCooldownUntil_.erase(command->name);
            }
        }

        if (core_ != nullptr) {
            switch (result) {
                case ohmycmd::MetadataResult::Ok:
                    core_->logLn(LogLevel::Debug, "[ohmycmd] cooldown set: /%s global=%dms player=%dms", name.c_str(), globalMs, playerMs);
                    break;
                case ohmycmd::MetadataResult::CommandNotFound:
                    core_->logLn(LogLevel::Warning, "[ohmycmd] cooldown set failed: command not found /%s", name.c_str());
                    break;
                case ohmycmd::MetadataResult::InvalidValue:
                    core_->logLn(LogLevel::Warning,
                                 "[ohmycmd] cooldown set failed: invalid values for /%s (global=%d player=%d)",
                                 name.c_str(),
                                 globalMs,
                                 playerMs);
                    break;
            }
        }

        return result == ohmycmd::MetadataResult::Ok ? 1 : 0;
    }

    cell nativeSetRateLimit(AMX* amx, cell* params) {
        const int argc = params[0] / static_cast<int>(sizeof(cell));
        if (argc < 1) {
            return 0;
        }

        const std::string name = readAmxString(amx, params[1]);
        const int maxCalls = argc >= 2 ? static_cast<int>(params[2]) : 0;
        const int windowMs = argc >= 3 ? static_cast<int>(params[3]) : 0;

        const ohmycmd::MetadataResult result = registry_.setRateLimit(name, maxCalls, windowMs);
        if (result == ohmycmd::MetadataResult::Ok) {
            if (const ohmycmd::CommandSpec* command = registry_.find(name); command != nullptr) {
                rateBuckets_.erase(command->name);
            }
        }

        if (core_ != nullptr) {
            switch (result) {
                case ohmycmd::MetadataResult::Ok:
                    core_->logLn(LogLevel::Debug, "[ohmycmd] rate limit set: /%s max=%d window=%dms", name.c_str(), maxCalls, windowMs);
                    break;
                case ohmycmd::MetadataResult::CommandNotFound:
                    core_->logLn(LogLevel::Warning, "[ohmycmd] rate limit set failed: command not found /%s", name.c_str());
                    break;
                case ohmycmd::MetadataResult::InvalidValue:
                    core_->logLn(LogLevel::Warning,
                                 "[ohmycmd] rate limit set failed: invalid values for /%s (max=%d window=%d)",
                                 name.c_str(),
                                 maxCalls,
                                 windowMs);
                    break;
            }
        }

        return result == ohmycmd::MetadataResult::Ok ? 1 : 0;
    }

    cell nativeExecute(AMX* amx, cell* params) {
        const int argc = params[0] / static_cast<int>(sizeof(cell));
        if (argc < 2 || core_ == nullptr) {
            return 0;
        }

        const int playerID = static_cast<int>(params[1]);
        const std::string input = readAmxString(amx, params[2]);

        IPlayer* player = core_->getPlayers().get(playerID);
        if (player == nullptr) {
            return 0;
        }

        const bool handled = onPlayerCommandText(*player, StringView(input.data(), input.size()));
        return handled ? 1 : 0;
    }

    cell nativeArgCount(AMX* amx, cell* params) {
        const int argc = params[0] / static_cast<int>(sizeof(cell));
        if (argc < 1) {
            return 0;
        }

        const std::string input = readAmxString(amx, params[1]);
        const std::vector<std::string> tokens = tokenizeArgs(input);
        return static_cast<cell>(tokens.size());
    }

    cell nativeArgInt(AMX* amx, cell* params) {
        const int argc = params[0] / static_cast<int>(sizeof(cell));
        if (argc < 3) {
            return 0;
        }

        const std::string input = readAmxString(amx, params[1]);
        const int index = static_cast<int>(params[2]);
        if (index < 0) {
            return 0;
        }

        const std::vector<std::string> tokens = tokenizeArgs(input);
        if (static_cast<size_t>(index) >= tokens.size()) {
            return 0;
        }

        int value = 0;
        if (!parseIntStrict(tokens[static_cast<size_t>(index)], value)) {
            return 0;
        }

        return writeAmxCell(amx, params[3], static_cast<cell>(value)) ? 1 : 0;
    }

    cell nativeArgFloat(AMX* amx, cell* params) {
        const int argc = params[0] / static_cast<int>(sizeof(cell));
        if (argc < 3) {
            return 0;
        }

        const std::string input = readAmxString(amx, params[1]);
        const int index = static_cast<int>(params[2]);
        if (index < 0) {
            return 0;
        }

        const std::vector<std::string> tokens = tokenizeArgs(input);
        if (static_cast<size_t>(index) >= tokens.size()) {
            return 0;
        }

        float value = 0.0F;
        if (!parseFloatStrict(tokens[static_cast<size_t>(index)], value)) {
            return 0;
        }

        return writeAmxCell(amx, params[3], amx_ftoc(value)) ? 1 : 0;
    }

    cell nativeArgPlayerID(AMX* amx, cell* params) {
        const int argc = params[0] / static_cast<int>(sizeof(cell));
        if (argc < 3 || core_ == nullptr) {
            return 0;
        }

        const std::string input = readAmxString(amx, params[1]);
        const int index = static_cast<int>(params[2]);
        if (index < 0) {
            return 0;
        }

        const std::vector<std::string> tokens = tokenizeArgs(input);
        if (static_cast<size_t>(index) >= tokens.size()) {
            return 0;
        }

        int playerID = 0;
        if (!parseIntStrict(tokens[static_cast<size_t>(index)], playerID)) {
            return 0;
        }

        if (core_->getPlayers().get(playerID) == nullptr) {
            return 0;
        }

        return writeAmxCell(amx, params[3], static_cast<cell>(playerID)) ? 1 : 0;
    }

    cell nativeArgString(AMX* amx, cell* params) {
        const int argc = params[0] / static_cast<int>(sizeof(cell));
        if (argc < 3) {
            return 0;
        }

        const std::string input = readAmxString(amx, params[1]);
        const int index = static_cast<int>(params[2]);
        if (index < 0) {
            return 0;
        }

        const size_t outputSize = argc >= 4 ? static_cast<size_t>(params[4]) : 1U;
        if (outputSize == 0) {
            return 0;
        }

        const std::vector<std::string> tokens = tokenizeArgs(input);
        if (static_cast<size_t>(index) >= tokens.size()) {
            (void)writeAmxString(amx, params[3], "", outputSize);
            return 0;
        }

        return writeAmxString(amx, params[3], tokens[static_cast<size_t>(index)], outputSize) ? 1 : 0;
    }

    cell nativeArgRest(AMX* amx, cell* params) {
        const int argc = params[0] / static_cast<int>(sizeof(cell));
        if (argc < 3) {
            return 0;
        }

        const std::string input = readAmxString(amx, params[1]);
        const int index = static_cast<int>(params[2]);
        if (index < 0) {
            return 0;
        }

        const size_t outputSize = argc >= 4 ? static_cast<size_t>(params[4]) : 1U;
        if (outputSize == 0) {
            return 0;
        }

        const std::vector<std::string> tokens = tokenizeArgs(input);
        if (static_cast<size_t>(index) >= tokens.size()) {
            (void)writeAmxString(amx, params[3], "", outputSize);
            return 0;
        }

        std::string rest;
        for (size_t i = static_cast<size_t>(index); i < tokens.size(); ++i) {
            if (!rest.empty()) {
                rest.push_back(' ');
            }
            rest.append(tokens[i]);
        }

        return writeAmxString(amx, params[3], rest, outputSize) ? 1 : 0;
    }

    DispatchResult invokePawnHandler(IPlayer& player, const ohmycmd::CommandSpec& command, const std::string& args) {
        if (pawnComponent_ == nullptr) {
            if (core_ != nullptr) {
                core_->logLn(LogLevel::Error, "[ohmycmd] dispatch failed: pawn component unavailable");
            }
            return DispatchResult::Failed;
        }

        IPawnScript* script = pawnComponent_->mainScript();
        if (script == nullptr || !script->IsLoaded()) {
            if (core_ != nullptr) {
                core_->logLn(LogLevel::Error, "[ohmycmd] dispatch failed: main script unavailable");
            }
            return DispatchResult::Failed;
        }

        int publicIndex = std::numeric_limits<int>::max();
        if (script->FindPublic(command.handlerPublic.c_str(), &publicIndex) != AMX_ERR_NONE || publicIndex == std::numeric_limits<int>::max()) {
            if (core_ != nullptr) {
                core_->logLn(LogLevel::Warning, "[ohmycmd] handler not found: %s", command.handlerPublic.c_str());
            }
            return DispatchResult::Failed;
        }

        cell retval = 0;
        const int err = script->CallChecked(publicIndex, retval, player.getID(), StringView(args.data(), args.size()));
        if (err != AMX_ERR_NONE) {
            script->PrintError(err);
            return DispatchResult::Failed;
        }

        if (core_ != nullptr) {
            core_->logLn(LogLevel::Debug,
                         "[ohmycmd] dispatched /%s -> %s (player=%d, handled=%d)",
                         command.name.c_str(),
                         command.handlerPublic.c_str(),
                         player.getID(),
                         retval != 0 ? 1 : 0);
        }

        return retval != 0 ? DispatchResult::CalledHandled : DispatchResult::CalledNotHandled;
    }

    PolicyDecision evaluatePolicy(IPawnScript* script, IPlayer& player, const ohmycmd::CommandSpec& command, TimePoint now) {
        if (script != nullptr && command.flags != 0) {
            const cell allowed = script->Call("OMC_OnCheckAccess",
                                              DefaultReturnValue_True,
                                              player.getID(),
                                              StringView(command.name.data(), command.name.size()),
                                              static_cast<int>(command.flags));

            if (allowed == 0) {
                return PolicyDecision { false, DenyReason::Permission, 0 };
            }
        }

        if (command.globalCooldownMs > 0) {
            const auto globalIt = globalCooldownUntil_.find(command.name);
            if (globalIt != globalCooldownUntil_.end() && globalIt->second > now) {
                return PolicyDecision { false, DenyReason::GlobalCooldown, remainingMs(now, globalIt->second) };
            }
        }

        if (command.playerCooldownMs > 0) {
            const auto commandIt = playerCooldownUntil_.find(command.name);
            if (commandIt != playerCooldownUntil_.end()) {
                const auto playerIt = commandIt->second.find(player.getID());
                if (playerIt != commandIt->second.end() && playerIt->second > now) {
                    return PolicyDecision { false, DenyReason::PlayerCooldown, remainingMs(now, playerIt->second) };
                }
            }
        }

        if (command.rateLimitCount > 0 && command.rateLimitWindowMs > 0) {
            const auto commandIt = rateBuckets_.find(command.name);
            if (commandIt != rateBuckets_.end()) {
                const auto playerIt = commandIt->second.find(player.getID());
                if (playerIt != commandIt->second.end()) {
                    const RateBucket& bucket = playerIt->second;
                    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - bucket.windowStart).count();
                    if (elapsedMs >= 0 && elapsedMs < command.rateLimitWindowMs && bucket.calls >= command.rateLimitCount) {
                        return PolicyDecision { false,
                                                DenyReason::RateLimit,
                                                command.rateLimitWindowMs - static_cast<int>(elapsedMs) };
                    }
                }
            }
        }

        return PolicyDecision {};
    }

    void consumeRateLimit(IPlayer& player, const ohmycmd::CommandSpec& command, TimePoint now) {
        if (command.rateLimitCount <= 0 || command.rateLimitWindowMs <= 0) {
            return;
        }

        auto& bucket = rateBuckets_[command.name][player.getID()];
        const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - bucket.windowStart).count();

        if (bucket.calls <= 0 || elapsedMs < 0 || elapsedMs >= command.rateLimitWindowMs) {
            bucket.windowStart = now;
            bucket.calls = 1;
            return;
        }

        ++bucket.calls;
    }

    void applyCooldown(IPlayer& player, const ohmycmd::CommandSpec& command, TimePoint now) {
        if (command.globalCooldownMs > 0) {
            globalCooldownUntil_[command.name] = now + std::chrono::milliseconds(command.globalCooldownMs);
        }

        if (command.playerCooldownMs > 0) {
            playerCooldownUntil_[command.name][player.getID()] = now + std::chrono::milliseconds(command.playerCooldownMs);
        }
    }

    void handlePolicyDenied(IPawnScript* script,
                            IPlayer& player,
                            const ohmycmd::CommandSpec& command,
                            DenyReason reason,
                            int retryMs) {
        if (script != nullptr) {
            const cell handled = script->Call("OMC_OnPolicyDeny",
                                              DefaultReturnValue_False,
                                              player.getID(),
                                              StringView(command.name.data(), command.name.size()),
                                              static_cast<int>(reason),
                                              retryMs);

            if (handled != 0) {
                return;
            }
        }

        switch (reason) {
            case DenyReason::Permission:
                player.sendClientMessage(Colour(255, 120, 120), "[ohmycmd] no permission.");
                break;
            case DenyReason::GlobalCooldown:
                player.sendClientMessage(Colour(255, 200, 120), "[ohmycmd] command is on global cooldown.");
                break;
            case DenyReason::PlayerCooldown:
                player.sendClientMessage(Colour(255, 200, 120), "[ohmycmd] command is on cooldown.");
                break;
            case DenyReason::RateLimit:
                player.sendClientMessage(Colour(255, 170, 120), "[ohmycmd] too many attempts, slow down.");
                break;
        }

        if (retryMs > 0 && (reason == DenyReason::GlobalCooldown || reason == DenyReason::PlayerCooldown || reason == DenyReason::RateLimit)) {
            std::string extra = "[ohmycmd] retry in " + std::to_string(retryMs) + "ms.";
            player.sendClientMessage(Colour(200, 200, 200), StringView(extra.data(), extra.size()));
        }
    }

    void sendCommandUsage(IPlayer& player, const ohmycmd::CommandSpec& command) {
        if (command.usage.empty()) {
            return;
        }

        std::string line = "[ohmycmd] usage: " + command.usage;
        player.sendClientMessage(Colour(180, 220, 255), StringView(line.data(), line.size()));
    }

    void sendCommandHelp(IPlayer& player, const ohmycmd::CommandSpec& command) {
        std::string head = "/" + command.name;
        if (!command.description.empty()) {
            head += " - ";
            head += command.description;
        }
        player.sendClientMessage(Colour(160, 220, 255), StringView(head.data(), head.size()));

        if (!command.usage.empty()) {
            std::string usage = "usage: " + command.usage;
            player.sendClientMessage(Colour(200, 200, 200), StringView(usage.data(), usage.size()));
        }

        if (!command.aliases.empty()) {
            std::string aliases = "aliases: ";
            for (size_t i = 0; i < command.aliases.size(); ++i) {
                if (i > 0) {
                    aliases += ", ";
                }
                aliases += "/";
                aliases += command.aliases[i];
            }
            player.sendClientMessage(Colour(200, 200, 200), StringView(aliases.data(), aliases.size()));
        }
    }

    bool isHelpRequest(std::string_view args) const {
        const std::string normalized = toLowerAscii(trim(args));
        return normalized == "help" || normalized == "?";
    }

    IPawnScript* mainScript() const {
        if (pawnComponent_ == nullptr) {
            return nullptr;
        }

        IPawnScript* script = pawnComponent_->mainScript();
        if (script == nullptr || !script->IsLoaded()) {
            return nullptr;
        }

        return script;
    }

    IPawnScript* scriptFromAmx(AMX* amx) const {
        if (amx == nullptr || pawnComponent_ == nullptr) {
            return nullptr;
        }

        return pawnComponent_->getScript(amx);
    }

    std::string readAmxString(AMX* amx, cell addr) const {
        IPawnScript* script = scriptFromAmx(amx);
        if (script == nullptr) {
            return {};
        }

        cell* physAddr = nullptr;
        if (script->GetAddr(addr, &physAddr) != AMX_ERR_NONE || physAddr == nullptr) {
            return {};
        }

        int len = 0;
        if (script->StrLen(physAddr, &len) != AMX_ERR_NONE || len <= 0) {
            return {};
        }

        std::string out(static_cast<size_t>(len) + 1, '\0');
        if (script->GetString(out.data(), physAddr, false, static_cast<size_t>(len) + 1) != AMX_ERR_NONE) {
            return {};
        }

        out.resize(static_cast<size_t>(len));
        return out;
    }

    bool writeAmxCell(AMX* amx, cell addr, cell value) const {
        IPawnScript* script = scriptFromAmx(amx);
        if (script == nullptr) {
            return false;
        }

        cell* physAddr = nullptr;
        if (script->GetAddr(addr, &physAddr) != AMX_ERR_NONE || physAddr == nullptr) {
            return false;
        }

        *physAddr = value;
        return true;
    }

    bool writeAmxString(AMX* amx, cell addr, std::string_view value, size_t maxCells) const {
        IPawnScript* script = scriptFromAmx(amx);
        if (script == nullptr || maxCells == 0) {
            return false;
        }

        cell* physAddr = nullptr;
        if (script->GetAddr(addr, &physAddr) != AMX_ERR_NONE || physAddr == nullptr) {
            return false;
        }

        return script->SetString(physAddr, StringView(value.data(), value.size()), false, false, maxCells) == AMX_ERR_NONE;
    }

    void detachEventHandlers() {
        if (core_ != nullptr && playerTextHooked_) {
            core_->getPlayers().getPlayerTextDispatcher().removeEventHandler(this);
            playerTextHooked_ = false;
        }

        if (pawnComponent_ != nullptr && pawnHooked_) {
            pawnComponent_->getEventDispatcher().removeEventHandler(this);
            pawnHooked_ = false;
        }
    }

private:
    ICore* core_ = nullptr;
    IComponentList* components_ = nullptr;
    IPawnComponent* pawnComponent_ = nullptr;

    bool playerTextHooked_ = false;
    bool pawnHooked_ = false;

    ohmycmd::CommandRegistry registry_;

    std::unordered_map<std::string, TimePoint> globalCooldownUntil_;
    std::unordered_map<std::string, std::unordered_map<int, TimePoint>> playerCooldownUntil_;
    std::unordered_map<std::string, std::unordered_map<int, RateBucket>> rateBuckets_;
};

} // namespace

COMPONENT_ENTRY_POINT() {
    return new OhMyCmdComponent();
}
