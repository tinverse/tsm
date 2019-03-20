#pragma once

#include "Event.h"
#include "State.h"
#include "Transition.h"

#include <set>
#include <unordered_map>
///
/// Captures 'structural' aspects of the state machine and behavior specific to
/// FsmDef. For e.g. the FsmDef can override the onEntry and onExit behaviors to
/// implement history preserving policy for specific events.
///
namespace tsm {

typedef std::pair<State&, Event> StateEventPair;

template<typename FsmDef>
struct FsmDefinition : public State
{
    using ActionFn = void (FsmDef::*)(void);
    using GuardFn = bool (FsmDef::*)(void);
    using Transition = TransitionT<State, Event, ActionFn, GuardFn>;
    using TransitionTableElement = std::pair<StateEventPair, Transition>;
    using TransitionTable =
      std::unordered_map<typename TransitionTableElement::first_type,
                         typename TransitionTableElement::second_type>;

    struct StateTransitionTable : private TransitionTable
    {
        using TransitionTable::end;
        using TransitionTable::find;
        using TransitionTable::insert;
        using TransitionTable::size;

      public:
        Transition* next(State& fromState, Event const& onEvent)
        {
            // Check if event in Hsm
            StateEventPair pair(fromState, onEvent);
            auto it = find(pair);
            if (it != end()) {
                return &it->second;
            }

            DLOG(ERROR) << "No Transition:" << fromState.name
                        << "\tonEvent:" << onEvent.id;
            return nullptr;
        }

        void print()
        {
            for (const auto& it : *this) {
                DLOG(INFO) << it.first.first->name << "," << it.first.second.id
                           << ":" << it.second->toState.name << "\n";
            }
        }
    };

    FsmDefinition(std::string const& name)
      : State(name)
      , currentState_(nullptr)
    {}

    virtual ~FsmDefinition() = default;

    void add(State& fromState,
             Event const& onEvent,
             State& toState,
             ActionFn action = nullptr,
             GuardFn guard = nullptr)
    {
        Transition t(fromState, onEvent, toState, action, guard);
        addTransition(fromState, onEvent, t);
        eventSet_.insert(onEvent);
    }

    Transition* next(State& currentState, Event const& nextEvent)
    {
        return table_.next(currentState, nextEvent);
    }

    State* getCurrentState()
    {
        DLOG(INFO) << "Get Current State: "
                   << ((currentState_) ? currentState_->name : "nullptr");
        return currentState_;
    }

    StateTransitionTable& getTable() const { return table_; }
    std::set<Event> const& getEvents() const { return eventSet_; }

    virtual State* getStartState() = 0;
    virtual State* getStopState() = 0;

  protected:
    StateTransitionTable table_;
    std::set<Event> eventSet_;
    State* currentState_;

  private:
    void addTransition(State& fromState,
                       Event const& onEvent,
                       Transition const& t)
    {
        StateEventPair pair(fromState, onEvent);
        TransitionTableElement e(pair, t);
        table_.insert(e);
    }
};
} // namespace tsm
