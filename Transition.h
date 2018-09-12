#pragma once

#include <glog/logging.h>
#include <memory>

using std::move;
using std::shared_ptr;

namespace tsm {

template<typename State, typename Event, typename ActionFn, typename GuardFn>
struct TransitionT
{
    TransitionT(shared_ptr<State> fromState,
                Event event,
                shared_ptr<State> toState,
                ActionFn action,
                GuardFn guard)
      : fromState(fromState)
      , onEvent(event)
      , toState(toState)
      , action(action)
      , guard(guard)
    {}

    virtual ~TransitionT() = default;

    template<typename DerivedHSM>
    ActionFn doTransition()
    {
        LOG(INFO) << __PRETTY_FUNCTION__;
        this->fromState->OnExit();
        this->toState->OnEntry();
        return action;
    }

    shared_ptr<State> fromState;
    Event onEvent;
    shared_ptr<State> toState;
    ActionFn action;
    GuardFn guard;
};

} // namespace tsm
