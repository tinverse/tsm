#pragma once

#include "Event.h"
#include "EventQueue.h"
#include "State.h"
#include "Transition.h"
#include "hash.h"

#include <atomic>
#include <future>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace tsm {

typedef TransitionT<State, Event> Transition;
typedef std::unordered_map<StateEventPair, Transition> TransitionTable;

// Forward declaration
class StateMachine;

class StateTransitionTable : private TransitionTable
{
    typedef typename ::std::pair<StateEventPair, Transition>
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

    std::shared_ptr<State> const& getCurrentState() const;

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

    auto& getParent() const { return parent_; }

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

        LOG(INFO) << "Invalidate State : " << this->name << std::endl;
        currentStatePromise_ = std::promise<std::shared_ptr<State>>();
    }

    auto getHsmSet() { return table_.getHsmSet(); }

    void determineParent();
};

} // namespace tsm
