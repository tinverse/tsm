#pragma once

#include "State.h"
#include "Event.h"
#include "EventQueue.h"
#include "Transition.h"

#include <atomic>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

typedef TransitionT<State, Event> Transition;
typedef std::pair<std::shared_ptr<State>, Event> StateEventPair;
#include "hash.h"

typedef std::unordered_map<StateEventPair, Transition> TransitionTable;
class StateTransitionTable : private TransitionTable
{
    typedef typename std::pair<StateEventPair, Transition>
        TransitionTableElement;

public:
    void add(std::shared_ptr<State> fromState, Event onEvent,
            std::shared_ptr<State> toState)
    {
      Transition t{fromState, onEvent, toState};
        StateEventPair pair(fromState, onEvent);
        TransitionTableElement e(pair, t);
        insert(e);
    }

    Transition* next(std::shared_ptr<State> fromState, Event onEvent);

     void print()
    {
        for (const auto& it: *this) {
            LOG(INFO) << it.first.first->name << "," << it.first.second.id << ":" << it.second.toState->name << "\n" ;
        }
    }
    size_t size() { return TransitionTable::size(); }

};

class StateMachine : public State
{
public:
    StateMachine() = delete;
    StateMachine(std::shared_ptr<State> startState,
                 std::shared_ptr<State> stopState,
                 EventQueue<Event>& eventQueue,
                 StateTransitionTable table,
                 std::shared_ptr<StateMachine> parent=nullptr)
              : interrupt(false)
              , currentState_(startState)
              , startState_(startState)
              , stopState_(stopState)
              , eventQueue_(eventQueue)
              , table_(table)
              , parent_(parent)
    {

        // initialize the state transition table.
        // This could be hard coded in this constructor or
        // better still, read from an input file.
        // All States should be created by the time we exit
        // the constructor.

    }
    virtual ~StateMachine() = default;

    auto getCurrentState() { return currentState_;  }
    auto getStartState() { return startState_;  }
    auto getStoptate() { return startState_;  }
    EventQueue<Event>& getEventQueue() { return eventQueue_;  }
    void start();

    void execute(void);

    void stop() { interrupt = true; eventQueue_.stop(); smThread_.join(); }

    virtual void OnEntry() { startState_->execute(); }
    // Data
    std::atomic<bool> interrupt;

protected:
    std::shared_ptr<State> currentState_;
    std::shared_ptr<State> startState_;
    std::shared_ptr<State> stopState_;
    EventQueue<Event>& eventQueue_;
    StateTransitionTable table_;
    std::shared_ptr<StateMachine> parent_;
    std::thread smThread_;


};
