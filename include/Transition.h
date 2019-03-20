#pragma once
#define CALL_MEMBER_FN(object, ptrToMember) ((object)->*(ptrToMember))()
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

        // Evaluate guard if it exists
        bool result = guard && CALL_MEMBER_FN(hsm, guard);

        if (!guard || result) {
            // Perform entry and exit actions in the doTransition function.
            // If just an internal transition, Entry and exit actions are
            // not performed

            this->fromState.onExit(onEvent);
            if (action) {
                CALL_MEMBER_FN(hsm, action);
            }
            this->toState.onEntry(onEvent);
        } else {
            DLOG(INFO) << "Guard prevented transition";
        }
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
    {}

    template<typename HsmType>
    void doTransition(HsmType* hsm)
    {
        // Evaluate guard if it exists
        bool result = this->guard && CALL_MEMBER_FN(hsm, this->guard);
        if (!(this->guard) || result) {

            if (this->action) {
                CALL_MEMBER_FN(hsm, this->action);
            }
        } else {
            DLOG(INFO) << "Guard prevented transition";
        }
    }
};

} // namespace tsm
