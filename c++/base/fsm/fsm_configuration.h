#pragma once
#include <functional>
#include <map>
namespace fsm {

using EntryFunction = std::function<void()>;
using EventFunction = std::function<void()>;
template <class StateEnum, class EventEnum, class Logger> class FsmConfiguration {
public:
    static FsmConfiguration &instance() {
        static FsmConfiguration inst;
        return inst;
    }

public:
    FsmConfiguration &entry(EntryFunction func) {
        return *this;
    }

    FsmConfiguration &Add() {
        return *this;
    }

    FsmConfiguration &WithInternal() {
        return *this;
    }

    FsmConfiguration &WithExternal() {
        return *this;
    }

    FsmConfiguration &SetSource(StateEnum state) {
        return *this;
    }

    FsmConfiguration &SetTarget(StateEnum state) {
        return *this;
    }

    FsmConfiguration &SetEvent(EventEnum state) {
        return *this;
    }

    FsmConfiguration &SetFunc(EventFunction func) {
        return *this;
    }

    FsmConfiguration &SetBeforeFunc(EventFunction func) {
        return *this;
    }

    FsmConfiguration &SetAfterFunc(EventFunction func) {
        return *this;
    }
};
} // namespace fsm