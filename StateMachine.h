#pragma once

#include "State.h"
#include "Event.h"
#include "EventQueue.h"
#include "Transition.h"

#include <atomic>
#include <memory>
#include <sstream>
#include <stdexcept>

typedef TransitionT<State, Event> Transition;
typedef std::pair<std::shared_ptr<State>, Event> StateEventPair;
typedef std::unordered_map<StateEventPair, Transition> TransitionTable;

class StateTransitionTable : private TransitionTable
{
    typedef typename std::pair<StateEventPair, Transition>
        TransitionTableElement;

public:
    void add(Transition t)
    {
        StateEventPair pair(t.fromState, t.onEvent);
        TransitionTableElement e(pair, t);
        insert(e);
    }

    Transition& next(std::shared_ptr<State> fromState, Event onEvent)
    {
        StateEventPair pair(fromState, onEvent);
        auto it = find(pair);
        if (it != end())
        {
            return it->second;
        }
        else
        {
            std::ostringstream s;
            s << "No Transition from State: " << fromState->id
              << " onEvent:" << onEvent.id << std::endl;
            throw std::logic_error(s.str());
        }
        return it->second;
    }
};

class StateMachine : public State
{
public:
    StateMachine(std::shared_ptr<State> startState,
                 std::shared_ptr<State> stopState,
                 EventQueue<Event>& eventQueue,
                 StateTransitionTable table)
        : running(false)
        , currentState_(startState)
        , startState_(startState)
        , stopState_(stopState)
        , eventQueue_(eventQueue)
        , table_(table)
    {

        // initialize the state transition table.
        // This could be hard coded in this constructor or
        // better still, read from an input file.
        // All States should be created by the time we exit
        // the constructor.

    }

    void start();

    void execute(void);

    void stop() { smThread_.join(); }

    virtual void OnEntry() { startState_->execute(); }
    // Data
    std::atomic<bool> running;

protected:
    std::shared_ptr<State> currentState_;
    std::shared_ptr<State> startState_;
    std::shared_ptr<State> stopState_;
    EventQueue<Event>& eventQueue_;
    StateTransitionTable table_;
    std::thread smThread_;
};
