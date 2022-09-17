//
// Created by xhou on 22-7-17.
//

#include "../fsm_statemachine.h"

using namespace fsm;

enum class CompState {
    COMP_NEW,
    COMP_INSTALLED,
    COMP_STARTING,
    COMP_RUNNING,
    COMP_STOPPING,
    COMP_STOPPED,
    COMP_FAULT,
    COMP_SUSPEND
};

enum class CompEvent {
    COMP_INSTALL,
    COMP_START,
    COMP_STOP,
    COMP_UNINSTALL,
    COMP_EXIT,
};

#define ENTRY_WRAPPER(func) [ this ]() { return func(); }

class CompBase : fsm::StateMachine<CompState, CompEvent, FLogger>, fsm::Context<CompState, CompEvent, FLogger> {
public:
    using FsmConfig = FsmConfiguration<CompState, CompEvent, FLogger>;

public:
    explicit CompBase(const std::string &name)
        : name_(name) {}

    uint32_t configure(FsmConfig &config) override { return 0; }

protected:
    std::string name_;
};

class CompNew : CompBase {
public:
    CompNew()
        : CompBase("new") {}

    uint32_t configure(FsmConfig &config) override {
        config.entry(ENTRY_WRAPPER(CompNew::OnEntry))
            .Add()
            .WithExternal()
            .SetSource(CompState::COMP_NEW)
            .SetEvent(CompEvent::COMP_INSTALL)
            .SetFunc(ENTRY_WRAPPER(CompNew::OnEventInstall));
        FLogger::log("configure for compNew\n");
        return 0;
    }

    uint32_t OnEntry() { return 0; }

    uint32_t OnEventInstall() { return 0; }
};

class CompReady : CompBase {
public:
    CompReady()
        : CompBase("ready") {}

    uint32_t configure(FsmConfig &config) override {
        config.entry(ENTRY_WRAPPER(CompReady::OnEntry))
            .Add()
            .WithInternal()
            .SetSource(CompState::COMP_INSTALLED)
            .SetEvent(CompEvent::COMP_INSTALL)
            .SetFunc(ENTRY_WRAPPER(CompReady::OnEventInstall))
            .Add()
            .WithExternal()
            .SetSource(CompState::COMP_INSTALLED)
            .SetTarget(CompState::COMP_STARTING)
            .SetEvent(CompEvent::COMP_START);
        FLogger::log("configure for compReady\n");
        return 0;
    }

    uint32_t OnEntry() {
        FLogger::log("log on Entry\n");
        return 0;
    }

    uint32_t OnEventInstall() {
        FLogger::log("log on Event Install Entry\n");
        return 0;
    }
};

FSM_REGISTER_STATE(CompNew);
FSM_REGISTER_STATE(CompReady);
int main(int argc, char **argv) {

    auto new_ = StateMachine<CompState, CompEvent, FLogger>::create();
    new_->initRootState(CompState::COMP_NEW);

    FLogger::log("main it\n");

    return 0;
}
