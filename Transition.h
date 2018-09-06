#pragma once

#include <glog/logging.h>
#include <memory>

using std::shared_ptr;
using std::move;

namespace tsm {

template<typename State, typename Event>
class TransitionT
{
  public:
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

    shared_ptr<State> fromState;
    shared_ptr<State> toState;
    Event onEvent;
};

} // namespace tsm
