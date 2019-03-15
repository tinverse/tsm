#pragma once

#include "Event.h"
#include "State.h"
#include "Transition.h"

#include <set>
#include <unordered_map>

namespace tsm {
typedef std::pair<State&, Event> StateEventPair;

///
/// Interface for any Hierarchical SM.
///
struct IHsmDef : public State
{
    IHsmDef() = delete;

    IHsmDef(std::string const& name, IHsmDef* parent)
      : State(name)
      , parent_(parent)
      , currentState_(nullptr)
      , currentHsm_(nullptr)
    {}

    virtual ~IHsmDef() = default;

    virtual State* getStartState() = 0;
    virtual State* getStopState() = 0;
    virtual IHsmDef* dispatch(IHsmDef*) = 0;

    State* getCurrentState()
    {
        DLOG(INFO) << "Get Current State: "
                   << ((currentState_) ? currentState_->name : "nullptr");
        return currentState_;
    }

    IHsmDef* getCurrentHsm() { return currentHsm_; }

    void setCurrentHsm(IHsmDef* currentHsm) { currentHsm_ = currentHsm; }
    IHsmDef* getParent() const { return parent_; }

    void setParent(IHsmDef* parent) { parent_ = parent; }

  protected:
    IHsmDef* parent_;
    State* currentState_;
    IHsmDef* currentHsm_;
};

///
/// Captures 'structural' aspects of the state machine and behavior specific to
/// HsmDef. For e.g. the HsmDef can override the onEntry and onExit behaviors to
/// implement history preserving policy for specific events.
///
template<typename HsmDef>
struct HsmDefinition : public IHsmDef
{
    using ActionFn = void (HsmDef::*)(void);
    using GuardFn = bool (HsmDef::*)(void);
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

    HsmDefinition() = delete;

    HsmDefinition(std::string const& name, IHsmDef* parent = nullptr)
      : IHsmDef(name, parent)
    {}

    virtual ~HsmDefinition() = default;

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

    StateTransitionTable& getTable() const { return table_; }
    std::set<Event> const& getEvents() const { return eventSet_; }

  protected:
    StateTransitionTable table_;
    std::set<Event> eventSet_;

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
