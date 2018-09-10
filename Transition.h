#pragma once

#include <glog/logging.h>
#include <memory>

using std::move;
using std::shared_ptr;

namespace tsm {

template<typename State, typename Event>
struct TransitionT
{
    TransitionT(shared_ptr<State> fromState,
                Event event,
                shared_ptr<State> toState)
      : fromState(move(fromState))
      , toState(move(toState))
      , onEvent(event)
    {}
    virtual ~TransitionT() = default;

    TransitionT(const TransitionT&) = default;

    virtual void doTransition()
    {
        fromState->OnExit();
        toState->OnEntry();
    }
    // TODO (sriram) : make protected
    shared_ptr<State> fromState;
    shared_ptr<State> toState;
    Event onEvent;
};

template<typename State, typename Event, typename Action>
struct TransitionWithAction : public TransitionT<State, Event>
{
    TransitionWithAction(shared_ptr<State> fromState,
                         Event event,
                         shared_ptr<State> toState,
                         Action action)
      : TransitionT<State, Event>(fromState, event, toState)
      , action(std::move(action))
    {}

    virtual ~TransitionWithAction() = default;

    void doTransition() override
    {
        LOG(INFO) << __PRETTY_FUNCTION__;
        this->fromState->OnExit();
        action(this->onEvent);
        this->toState->OnEntry();
    }
    Action action;
};

template<typename State, typename Event, typename Action, typename Guard>
struct GuardedTransitionWithAction
  : public TransitionWithAction<State, Event, Action>
{
    GuardedTransitionWithAction(shared_ptr<State> fromState,
                                Event event,
                                shared_ptr<State> toState,
                                Action action,
                                Guard guard)
      : TransitionWithAction<State, Event, Action>(fromState,
                                                   event,
                                                   toState,
                                                   action)
      , guard(std::move(guard))
    {}

    void doTransition() override
    {
        if (guard()) {
            TransitionWithAction<State, Event, Action>::doTransition();
        }
    }

    Guard guard;
};

auto EmptyAction = []() {};

template<typename State, typename Event, typename Guard>
struct GuardedTransition
  : public GuardedTransitionWithAction<State,
                                       Event,
                                       decltype(EmptyAction),
                                       Guard>
{
    GuardedTransition(shared_ptr<State> fromState,
                      Event event,
                      shared_ptr<State> toState,
                      Guard guard)
      : GuardedTransitionWithAction<State, Event, decltype(EmptyAction), Guard>(
          fromState,
          event,
          toState,
          EmptyAction,
          guard)
    {}
};

} // namespace tsm
