#pragma once
#include "hash.h"

#include <glog/logging.h>
#include <iostream>
#include <memory>
#include <unordered_map>

template <typename State, typename Event> class TransitionT
{
public:
    TransitionT(std::shared_ptr<State> fromState,
                Event event,
                std::shared_ptr<State> toState)
        : fromState(fromState)
        , toState(toState)
        , onEvent(event)
    {
    }
    virtual ~TransitionT()          = default;

    TransitionT(const TransitionT&) = default;

    virtual void execute() { DLOG(INFO) << __PRETTY_FUNCTION__ << std::endl; }

    std::shared_ptr<State> fromState;
    std::shared_ptr<State> toState;
    Event onEvent;
};



