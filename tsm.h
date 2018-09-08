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
#include <unordered_map>

using std::shared_ptr;

namespace tsm {

typedef TransitionT<State, Event> Transition;

// typedef TransitionA<State, Event, Action> TransitionWithAction;

typedef std::unordered_map<StateEventPair, shared_ptr<Transition>>
  TransitionTable;

// Forward declaration
struct StateMachine;

struct StateTransitionTable : private TransitionTable
{
    typedef typename ::std::pair<StateEventPair, shared_ptr<Transition>>
      TransitionTableElement;

  public:
    void add(shared_ptr<State> fromState,
             Event onEvent,
             shared_ptr<State> toState);

    template<typename Action>
    void add(shared_ptr<State> fromState,
             Event onEvent,
             shared_ptr<State> toState,
             Action action)
    {
        shared_ptr<Transition> t =
          std::make_shared<TransitionWithAction<State, Event, Action>>(
            fromState, onEvent, toState, action);
        StateEventPair pair(fromState, onEvent);
        TransitionTableElement e(pair, t);
        insert(e);

        // If HSM, add to hsm set.
        auto hsm = std::dynamic_pointer_cast<StateMachine>(fromState);
        if (hsm != nullptr) {
            hsmSet_.insert(hsm);
        }
    }

    shared_ptr<Transition> next(shared_ptr<State> fromState, Event onEvent);

    void print();

    size_t size() { return TransitionTable::size(); }

    auto& getHsmSet() { return hsmSet_; }

  private:
    std::set<shared_ptr<StateMachine>> hsmSet_;
};

struct StateMachine : public State
{
  public:
    StateMachine() = delete;

    StateMachine(std::string name,
                 shared_ptr<State> startState,
                 shared_ptr<State> stopState,
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
        // In-order traverse all states. If HSM found, set its parent
        determineParent();
    }

    virtual ~StateMachine() override = default;

    virtual shared_ptr<State> const& getCurrentState() const;

    void start();

    void OnEntry() override
    {
        DLOG(INFO) << "Entering: " << this->name;
        // Stopping a HSM means stopping all of its sub HSMs
        for (auto& hsm : table_.getHsmSet()) {
            hsm->OnEntry();
        }

        start();
    }

    void execute() override;

    void OnExit() override
    {
        // Stopping a HSM means stopping all of its sub HSMs
        LOG(INFO) << "Exiting: " << name;
        for (auto& hsm : table_.getHsmSet()) {
            hsm->OnExit();
        }
        stop();
    }

    void stop();

    void setParent(StateMachine* parent) { parent_ = parent; }

    auto& getParent() const { return parent_; }

    auto& getTable() const { return table_; }

  protected:
    std::atomic<bool> interrupt_;
    shared_ptr<State> currentState_;
    shared_ptr<State> startState_;
    shared_ptr<State> stopState_;
    EventQueue<Event>& eventQueue_;
    StateTransitionTable table_;
    StateMachine* parent_;
    std::thread smThread_;

  private:
    auto getHsmSet() { return table_.getHsmSet(); }

    void determineParent();
};

} // namespace tsm
