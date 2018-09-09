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
struct StateMachine : public State
{
    typedef TransitionT<State, Event> Transition;

    typedef std::unordered_map<StateEventPair, shared_ptr<Transition>>
      TransitionTable;

    struct StateTransitionTable : private TransitionTable
    {
        using TransitionTable::insert;

      public:
        shared_ptr<Transition> next(shared_ptr<State> fromState, Event onEvent);

        void print();

        size_t size() { return TransitionTable::size(); }
    };

    typedef typename ::std::pair<StateEventPair, shared_ptr<Transition>>
      TransitionTableElement;

  public:
    StateMachine() = delete;

    StateMachine(std::string name,
                 shared_ptr<State> startState,
                 shared_ptr<State> stopState,
                 EventQueue<Event>& eventQueue)
      : State(name)
      , interrupt_(false)
      , currentState_(nullptr)
      , startState_(startState)
      , stopState_(std::move(stopState))
      , eventQueue_(eventQueue)
      , parent_(nullptr)
    {}

    virtual ~StateMachine() override = default;

    auto& getParent() const { return parent_; }

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
        addTransition(fromState, onEvent, t);
    }
    virtual shared_ptr<State> const& getCurrentState() const;

    void start();

    void OnEntry() override
    {
        DLOG(INFO) << "Entering: " << this->name;
        // Stopping a HSM means stopping all of its sub HSMs
        for (auto& hsm : hsmSet_) {
            hsm->OnEntry();
        }

        start();
    }

    void execute() override;

    void OnExit() override
    {
        // Stopping a HSM means stopping all of its sub HSMs
        LOG(INFO) << "Exiting: " << name;
        for (auto& hsm : hsmSet_) {
            hsm->OnExit();
        }
        stop();
    }

    void stop();

    void setParent(StateMachine* parent) { parent_ = parent; }

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
    void addTransition(shared_ptr<State> fromState,
                       Event onEvent,
                       shared_ptr<Transition> t);

    std::set<shared_ptr<StateMachine>> hsmSet_;
};

} // namespace tsm
