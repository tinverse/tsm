#pragma once

#include "Event.h"
#include "EventQueue.h"
#include "State.h"
#include "Transition.h"

#include <atomic>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <future>

typedef TransitionT<State, Event> Transition;
typedef std::pair<std::shared_ptr<State>, Event> StateEventPair;
#include "hash.h"

typedef std::unordered_map<StateEventPair, Transition> TransitionTable;
class StateTransitionTable : private TransitionTable
{
    typedef typename std::pair<StateEventPair, Transition>
        TransitionTableElement;

public:
    void add(std::shared_ptr<State> fromState,
             Event onEvent,
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
        for (const auto& it : *this)
        {
            LOG(INFO) << it.first.first->name << "," << it.first.second.id
                      << ":" << it.second.toState->name << "\n";
        }
    }
    size_t size() { return TransitionTable::size(); }
};

class StateMachine : public State
{
public:
    StateMachine() = delete;

    StateMachine(std::string name,
            std::shared_ptr<State> startState,
                 std::shared_ptr<State> stopState,
                 StateTransitionTable table,
                 std::shared_ptr<StateMachine> parent = nullptr)
        : State(name)
        , interrupt_(false)
        , currentState_(nullptr)
        , startState_(startState)
        , stopState_(std::move(stopState))
        , table_(std::move(table))
        , parent_(std::move(parent))
    {

        // initialize the state transition table.
        // This could be hard coded in this constructor or
        // better still, read from an input file.
        // All States should be created by the time we exit
        // the constructor.
    }
    ~StateMachine() override = default;

    auto getCurrentState() { return currentStatePromise_.get_future().get(); }
    auto getStartState() { return startState_; }
    auto getStoptate() { return startState_; }

    void addEvent(Event const & e) {
        invalidateCurrentState();
        eventQueue_.addEvent(e);
    }
    void addFront(Event const & e) {
        invalidateCurrentState();
        eventQueue_.addFront(e);
    }
    void start();

    void execute(void) override;

    void stop()
    {
        interrupt_ = true;
        eventQueue_.stop();
        smThread_.join();
    }

    void OnEntry() override { startState_->execute(); }

protected:
    std::atomic<bool> interrupt_;
    std::promise< std::shared_ptr<State> > currentStatePromise_;
    std::shared_ptr<State> currentState_;
    std::shared_ptr<State> startState_;
    std::shared_ptr<State> stopState_;
    EventQueue<Event> eventQueue_;
    StateTransitionTable table_;
    std::shared_ptr<StateMachine> parent_;
    std::thread smThread_;
private:
    void invalidateCurrentState() {
        currentStatePromise_ = std::promise< std::shared_ptr<State> >();
    }
};
