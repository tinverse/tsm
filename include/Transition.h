#pragma once

namespace tsm {

template<typename State, typename Event, typename ActionFn, typename GuardFn>
struct TransitionT
{
    TransitionT(State& fromState,
                Event const& event,
                State& toState,
                ActionFn action,
                GuardFn guard)
      : fromState(fromState)
      , onEvent(event)
      , toState(toState)
      , action(action)
      , guard(guard)
    {}

    virtual ~TransitionT() = default;

    template<typename HsmType>
    void doTransition(HsmType* hsm)
    {
        if (!hsm) {
            // throw NullPointerException;
        }

        this->fromState.onExit(onEvent);
        if (action) {
            (hsm->*action)();
        }
        this->toState.onEntry(onEvent);
    }

    State& fromState;
    Event onEvent;
    State& toState;
    ActionFn action;
    GuardFn guard;
};

template<typename State, typename Event, typename ActionFn, typename GuardFn>
struct InternalTransitionT : public TransitionT<State, Event, ActionFn, GuardFn>
{
    InternalTransitionT(State& fromState,
                        Event const& event,
                        State& toState,
                        ActionFn action,
                        GuardFn guard)
      : TransitionT<State, Event, ActionFn, GuardFn>(fromState,
                                                     event,
                                                     fromState,
                                                     action,
                                                     guard)
      , action(action)
    {}

    template<typename HsmType>
    void doTransition(HsmType* hsm)
    {
        if (action) {
            (hsm->*action)();
        }
    }
    ActionFn action;
};

} // namespace tsm
