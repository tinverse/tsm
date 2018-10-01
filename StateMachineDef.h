#pragma once

#include "Event.h"
#include "State.h"
#include "Transition.h"

#include <set>
#include <unordered_map>

namespace tsm {
typedef std::pair<std::shared_ptr<State>, Event> StateEventPair;

template<typename HSMDef>
struct StateMachineDef : public State
{
    using ActionFn = void (HSMDef::*)(void);
    using GuardFn = bool (HSMDef::*)(void);
    using Transition = TransitionT<State, Event, ActionFn, GuardFn>;
    using TransitionTable =
      std::unordered_map<StateEventPair, shared_ptr<Transition>>;
    using TransitionTableElement =
      std::pair<StateEventPair, shared_ptr<Transition>>;

    struct StateTransitionTable : private TransitionTable
    {
        using TransitionTable::end;
        using TransitionTable::find;
        using TransitionTable::insert;
        using TransitionTable::size;

      public:
        shared_ptr<Transition> next(shared_ptr<State> fromState,
                                    Event const& onEvent)
        {
            // Check if event in HSM
            StateEventPair pair(fromState, onEvent);
            auto it = find(pair);
            if (it != end()) {
                return it->second;
            }

            LOG(ERROR) << "No Transition:" << fromState->name
                       << "\tonEvent:" << onEvent.id;
            return nullptr;
        }

        void print()
        {
            for (const auto& it : *this) {
                LOG(INFO) << it.first.first->name << "," << it.first.second.id
                          << ":" << it.second->toState->name << "\n";
            }
        }
    };

    StateMachineDef() = delete;

    StateMachineDef(std::string const& name,
                    shared_ptr<State> stopState,
                    State* parent = nullptr)
      : State(name)
      , stopState_(std::move(stopState))
      , parent_(parent)
      , currentState_(nullptr)
    {}

    StateMachineDef(std::string const& name, State* parent = nullptr)
      : StateMachineDef(name, nullptr, parent)
    {}

    void add(shared_ptr<State> fromState,
             Event const& onEvent,
             shared_ptr<State> toState,
             ActionFn action = nullptr,
             GuardFn guard = nullptr)
    {

        shared_ptr<Transition> t = std::make_shared<Transition>(
          fromState, onEvent, toState, action, guard);
        addTransition(fromState, onEvent, t);
        eventSet_.insert(onEvent);
    }

    shared_ptr<Transition> next(shared_ptr<State> currentState,
                                Event const& nextEvent)
    {
        return table_.next(currentState, nextEvent);
    }

    void onEntry(Event const&) override
    {
        DLOG(INFO) << "Entering: " << this->name;
        currentState_ = static_cast<HSMDef*>(this)->getStartState();
    }

    void onExit(Event const&) override
    {
        // TODO (sriram): Does the sub-HSM remember which state it was in at
        // exit? This really depends on exit/history policy. Sometimes you want
        // to retain state information when you exit a sub-HSM for certain
        // events. Maybe adding a currenEvent_ variable would allow HSMDefs to
        // override onExit appropriately.
        // Currently as you see, the policy is to 'forget' on exit by setting
        // the currentState_ to nullptr.
        DLOG(INFO) << "Exiting: " << this->name;
        currentState_ = nullptr;
    }

    auto& getTable() const { return table_; }
    auto& getEvents() const { return eventSet_; }

    virtual shared_ptr<State> getStopState() const { return stopState_; }

    State* getParent() const override { return parent_; }
    void setParent(State* parent) { parent_ = parent; }

  protected:
    StateTransitionTable table_;
    shared_ptr<State> stopState_;
    State* parent_;
    std::set<Event> eventSet_;
    shared_ptr<State> currentState_;

  private:
    void addTransition(shared_ptr<State> fromState,
                       Event onEvent,
                       shared_ptr<Transition> t)
    {
        StateEventPair pair(fromState, onEvent);
        TransitionTableElement e(pair, t);
        table_.insert(e);
    }
};
} // namespace tsm
