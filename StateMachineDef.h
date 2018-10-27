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

template<typename HSMDef>
struct StateMachineDef : public IHsmDef
{
    using ActionFn = void (HSMDef::*)(void);
    using GuardFn = bool (HSMDef::*)(void);
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
            // Check if event in HSM
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

    StateMachineDef() = delete;

    StateMachineDef(std::string const& name, IHsmDef* parent = nullptr)
      : IHsmDef(name, parent)
    {}

    virtual ~StateMachineDef() = default;

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

    void onEntry(Event const& e) override
    {
        DLOG(INFO) << "Entering: " << this->name;
        currentState_ = this->getStartState();

        if (this->parent_) {
            this->parent_->setCurrentHsm(this);
        }

        this->currentState_->execute(e);
    }

    void onExit(Event const&) override
    {
        // TODO (sriram): Does the sub-HSM remember which state it was in at
        // exit? This really depends on exit/history policy. Sometimes you
        // want to retain state information when you exit a sub-HSM for
        // certain events. Maybe adding a currenEvent_ variable would allow
        // HSMDefs to override onExit appropriately. Currently as you see,
        // the policy is to 'forget' on exit by setting the currentState_ to
        // nullptr.
        DLOG(INFO) << "Exiting: " << this->name;
        this->currentState_ = nullptr;

        if (this->parent_) {
            this->parent_->setCurrentHsm(nullptr);
        }
    }

    auto& getTable() const { return table_; }
    auto& getEvents() const { return eventSet_; }

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
