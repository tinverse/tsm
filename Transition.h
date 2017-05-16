#pragma once
#include "Event.h"
#include "State.h"
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
    TransitionT(const TransitionT&) = default;
    virtual ~TransitionT()          = default;

    std::shared_ptr<State> fromState;
    std::shared_ptr<State> toState;
    Event onEvent;
};

class Transition : public TransitionT<State, Event>
{

public:
    Transition(std::shared_ptr<State> fromState,
               Event event,
               std::shared_ptr<State> toState)
        : TransitionT(fromState, event, toState)
    {
    }

    Transition(const Transition&) = default;
    virtual ~Transition()         = default;

    virtual void execute() { DLOG(INFO) << __PRETTY_FUNCTION__ << std::endl; }
};

typedef std::pair<std::shared_ptr<State>, Event> StateEventPair;
typedef std::unordered_map<StateEventPair, Transition> TransitionTable;

inline bool operator==(const StateEventPair& p1, const StateEventPair& p2)
{
    return (p1.first->id == p2.first->id) && (p1.second.id == p2.second.id);
}
