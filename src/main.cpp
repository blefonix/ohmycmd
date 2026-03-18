#include <sdk.hpp>

#include <memory>

class OhMyCmdComponent final : public IComponent {
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
        core_->printLn("[ohmycmd] onLoad: component loaded");
    }

    void onInit(IComponentList* components) override {
        components_ = components;
        if (core_ != nullptr) {
            core_->printLn("[ohmycmd] onInit: components wired");
        }
    }

    void onReady() override {
        if (core_ != nullptr) {
            core_->printLn("[ohmycmd] onReady: ready");
        }
    }

    void onFree(IComponent* component) override {
        if (core_ != nullptr && component != nullptr) {
            StringView name = component->componentName();
            core_->logLn(LogLevel::Debug, "[ohmycmd] onFree: component released: %.*s", static_cast<int>(name.length()), name.data());
        }
    }

    void reset() override {
        if (core_ != nullptr) {
            core_->printLn("[ohmycmd] reset");
        }
    }

    void free() override {
        if (core_ != nullptr) {
            core_->printLn("[ohmycmd] free");
        }
        delete this;
    }

private:
    ICore* core_ = nullptr;
    IComponentList* components_ = nullptr;
};

COMPONENT_ENTRY_POINT() {
    return new OhMyCmdComponent();
}
