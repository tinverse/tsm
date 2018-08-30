#pragma once

#include "Event.h"
#include "EventQueue.h"
#include "State.h"
#include "Transition.h"

#include <atomic>
#include <future>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

typedef TransitionT<State, Event> Transition;

typedef std::pair<std::shared_ptr<State>, Event> StateEventPair;

#include "hash.h"

typedef std::unordered_map<StateEventPair, Transition> TransitionTable;

// Forward declaration
class StateMachine;

class StateTransitionTable : private TransitionTable
{
    typedef typename std::pair<StateEventPair, Transition>
      TransitionTableElement;

  public:
    void add(std::shared_ptr<State> fromState,
             Event onEvent,
             std::shared_ptr<State> toState);

    Transition* next(std::shared_ptr<State> fromState, Event onEvent);

    void print();

    size_t size() { return TransitionTable::size(); }

    auto& getHsmSet() { return hsmSet_; }

  private:
    std::set<std::shared_ptr<StateMachine>> hsmSet_;
};

#include <iostream>
class StateMachine : public State
{
  public:
    StateMachine() = delete;

    StateMachine(std::string name,
                 std::shared_ptr<State> startState,
                 std::shared_ptr<State> stopState,
                 EventQueue<Event>& eventQueue,
                 StateTransitionTable table)
      : State(name)
      , interrupt_(false)
      , currentState_(nullptr)
      , startState_(startState)
      , stopState_(std::move(stopState))
      , eventQueue_(eventQueue)
      , table_(std::move(table))
      , parent_(nullptr)
    {

        // initialize the state transition table.
        // This could be hard coded in this constructor or
        // better still, read from an input file.
        // All States should be created by the time we exit
        // the constructor.

        // In-order traverse all states. If HSM found, set its parent
        determineParent();
    }

    virtual ~StateMachine() override = default;

    auto getCurrentState()
    {
        if (!eventQueue_.empty()) {
            invalidateCurrentState();
        }
        return currentStatePromise_.get_future().get();
    }

    void addFront(Event const& e)
    {
        invalidateCurrentState();
        eventQueue_.addFront(e);
    }

    void start();

    void OnEntry() override
    {
        DLOG(INFO) << "Entering: " << this->name << std::endl;
        start();
    }

    void execute() override;

    void OnExit() override
    {
        DLOG(INFO) << "Exiting: " << this->name << std::endl;
        stop();
    }

    void stop();

    void setParent(StateMachine* parent) { parent_ = parent; }

    auto getParent() const { return parent_; }

    auto& getTable() const { return table_; }

  protected:
    std::atomic<bool> interrupt_;
    std::promise<std::shared_ptr<State>> currentStatePromise_;
    std::shared_ptr<State> currentState_;
    std::shared_ptr<State> startState_;
    std::shared_ptr<State> stopState_;
    EventQueue<Event>& eventQueue_;
    StateTransitionTable table_;
    StateMachine* parent_;
    std::thread smThread_;

  private:
    void invalidateCurrentState()
    {
        currentStatePromise_ = std::promise<std::shared_ptr<State>>();
    }

    auto getHsmSet() { return table_.getHsmSet(); }

    void determineParent();
};
