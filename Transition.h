#pragma once

#include <glog/logging.h>
#include <memory>

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

    template<typename HSMType>
    void doTransition(HSMType* hsm)
    {
        if (!hsm) {
            // throw NullPointerException;
        }

        this->fromState->onExit();
        if (action) {
            (hsm->*action)();
        }
        this->toState->onEntry();
    }

    shared_ptr<State> fromState;
    Event onEvent;
    shared_ptr<State> toState;
    ActionFn action;
    GuardFn guard;
};

template<typename State, typename Event, typename ActionFn, typename GuardFn>
struct InternalTransitionT : public TransitionT<State, Event, ActionFn, GuardFn>
{
    InternalTransitionT(shared_ptr<State> fromState,
                        Event event,
                        shared_ptr<State> toState,
                        ActionFn action,
                        GuardFn guard)
      : TransitionT<State, Event, ActionFn, GuardFn>(fromState,
                                                     event,
                                                     fromState,
                                                     action,
                                                     guard)
      , action(action)
    {}

    template<typename HSMType>
    void doTransition(HSMType* hsm)
    {
        if (action) {
            (hsm->*action)();
        }
    }
    ActionFn action;
};

} // namespace tsm
