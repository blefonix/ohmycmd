#include <sdk.hpp>
#include <Server/Components/Pawn/pawn.hpp>

#include "command_parser.hpp"
#include "command_registry.hpp"

#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

namespace {

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
        if (core_ != nullptr) {
            core_->printLn("[ohmycmd] reset: registry cleared");
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

        const bool executed = invokePawnHandler(player, *command, parsed.arguments);
        if (!executed) {
            player.sendClientMessage(Colour(255, 80, 80), "[ohmycmd] handler failed. Check server logs.");
        }

        // command was resolved by ohmycmd -> consume
        return true;
    }

    // PawnEventHandler
    void onAmxLoad(IPawnScript& script) override {
        static const std::array<AMX_NATIVE_INFO, 6> kNatives = {{
            { "OhmyCmd_Register", &OhMyCmdComponent::Native_OhmyCmd_Register },
            { "OhmyCmd_AddAlias", &OhMyCmdComponent::Native_OhmyCmd_AddAlias },
            { "OhmyCmd_SetDescription", &OhMyCmdComponent::Native_OhmyCmd_SetDescription },
            { "OhmyCmd_SetUsage", &OhMyCmdComponent::Native_OhmyCmd_SetUsage },
            { "OhmyCmd_Execute", &OhMyCmdComponent::Native_OhmyCmd_Execute },
            { "OhmyCmd_Count", &OhMyCmdComponent::Native_OhmyCmd_Count },
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
            if (core_ != nullptr) {
                core_->logLn(LogLevel::Debug, "[ohmycmd] main script unloaded: registry cleared");
            }
        }
    }

private:
    static cell Native_OhmyCmd_Register(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeRegister(amx, params) : 0;
    }

    static cell Native_OhmyCmd_AddAlias(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeAddAlias(amx, params) : 0;
    }

    static cell Native_OhmyCmd_SetDescription(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeSetDescription(amx, params) : 0;
    }

    static cell Native_OhmyCmd_SetUsage(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeSetUsage(amx, params) : 0;
    }

    static cell Native_OhmyCmd_Execute(AMX* amx, cell* params) {
        return g_instance != nullptr ? g_instance->nativeExecute(amx, params) : 0;
    }

    static cell Native_OhmyCmd_Count(AMX* amx, cell* params) {
        (void)amx;
        (void)params;
        return g_instance != nullptr ? static_cast<cell>(g_instance->registry_.size()) : 0;
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

    bool invokePawnHandler(IPlayer& player, const ohmycmd::CommandSpec& command, const std::string& args) {
        if (pawnComponent_ == nullptr) {
            if (core_ != nullptr) {
                core_->logLn(LogLevel::Error, "[ohmycmd] dispatch failed: pawn component unavailable");
            }
            return false;
        }

        IPawnScript* script = pawnComponent_->mainScript();
        if (script == nullptr || !script->IsLoaded()) {
            if (core_ != nullptr) {
                core_->logLn(LogLevel::Error, "[ohmycmd] dispatch failed: main script unavailable");
            }
            return false;
        }

        int publicIndex = std::numeric_limits<int>::max();
        if (script->FindPublic(command.handlerPublic.c_str(), &publicIndex) != AMX_ERR_NONE || publicIndex == std::numeric_limits<int>::max()) {
            if (core_ != nullptr) {
                core_->logLn(LogLevel::Warning, "[ohmycmd] handler not found: %s", command.handlerPublic.c_str());
            }
            return false;
        }

        cell retval = 0;
        const int err = script->CallChecked(publicIndex, retval, player.getID(), StringView(args.data(), args.size()));
        if (err != AMX_ERR_NONE) {
            script->PrintError(err);
            return false;
        }

        if (core_ != nullptr) {
            core_->logLn(LogLevel::Debug,
                         "[ohmycmd] dispatched /%s -> %s (player=%d, handled=%d)",
                         command.name.c_str(),
                         command.handlerPublic.c_str(),
                         player.getID(),
                         retval != 0 ? 1 : 0);
        }

        return true;
    }

    std::string readAmxString(AMX* amx, cell addr) const {
        if (amx == nullptr || pawnComponent_ == nullptr) {
            return {};
        }

        IPawnScript* script = pawnComponent_->getScript(amx);
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
};

} // namespace

COMPONENT_ENTRY_POINT() {
    return new OhMyCmdComponent();
}
