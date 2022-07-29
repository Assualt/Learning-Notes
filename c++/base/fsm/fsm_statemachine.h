#pragma once
#include "fsm_configuration.h"
#include "fsm_context.h"
#include "fsm_logger.h"
#include <memory>

namespace fsm {

enum ERRCODE { FSM_SUCCESS = 0, FSM_FAILURE };

template <class StateEnum, class EventEnum, class Logger> class StateMachine {
public:
    using FsmConfig  = FsmConfiguration<StateEnum, EventEnum, Logger>;
    using FsmContext = Context<StateEnum, EventEnum, Logger>;

public:
    static std::shared_ptr<StateMachine> create() {
        auto state   = std::make_shared<StateMachine>();
        auto context = std::make_shared<FsmContext>();
        auto ret     = state->initialize();
        if (ret != true) {
            Logger::log("initialize failed for context");
            return nullptr;
        }

        return state;
    }

    bool initialize() {
        auto cfg = std::make_shared<FsmConfig>();
        configure(*cfg);
        return true;
    }

    virtual uint32_t configure(FsmConfig &cfg) {
        return 0;
    }

    void initRootState(StateEnum state) {
    }
};

#define FSM_REGISTER_STATE(name) \
    static __attribute((constructor)) void __fsmConfig##name() { printf("before main\n"); }
} // namespace fsm