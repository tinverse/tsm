#pragma once

#include <glog/logging.h>
#include <iostream>
#include <memory>

template<typename State, typename Event>
class TransitionT
{
  public:
    TransitionT(std::shared_ptr<State> fromState,
                Event event,
                std::shared_ptr<State> toState)
      : fromState(std::move(fromState))
      , toState(std::move(toState))
      , onEvent(event)
    {}
    virtual ~TransitionT() = default;

    TransitionT(const TransitionT&) = default;

    virtual void doTransition()
    {
        fromState->OnExit();
        toState->OnEntry();
    }

    std::shared_ptr<State> fromState;
    std::shared_ptr<State> toState;
    Event onEvent;
};
