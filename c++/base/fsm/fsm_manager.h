#pragma once

namespace fsm {

template <class Event, class State, class Log> class FsmManager {
public:
    FsmManager &configure() {
        return *this;
    }

private:
    Event event_;
    State state_;
};

} // namespace fsm