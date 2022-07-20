#pragma once
#ifndef CALL_MEMBER_FN
#define CALL_MEMBER_FN(object, ptrToMember) ((object)->*(ptrToMember))()
#endif
#include "Event.h"
#include "State.h"

#include <set>
#include <unordered_map>
namespace tsm {

template<typename FsmDef,
         typename ActionFnT = void (FsmDef::*)(),
         typename GuardFnT = bool (FsmDef::*)()>
struct StateTransitionTableT
{
    using ActionFn = ActionFnT;
    using GuardFn = GuardFnT;

    struct Transition
    {
        Transition(State& toState, ActionFn action, GuardFn guard)
          : toState(toState)
          , action(action)
          , guard(guard)
        {}

        bool doTransition(FsmDef* hsm, Event const& e)
        {
            bool transitioned = false;

            // Evaluate guard if it exists
            bool result = guard && CALL_MEMBER_FN(hsm, guard);

            if (!guard || result) {
                // Perform entry and exit actions in the doTransition function.
                // If just an internal transition, Entry and exit actions are
                // not performed

                hsm->getCurrentState()->onExit(e);
                if (action) {
                    CALL_MEMBER_FN(hsm, action);
                }
                hsm->setCurrentState(&toState);
                this->toState.onEntry(e);
                transitioned = true;
            }
            return transitioned;
        }
        State& toState;
        ActionFn action;
        GuardFn guard;
    };

    struct InternalTransition : public Transition
    {
        InternalTransition(ActionFn action, GuardFn guard)
          : Transition(action, guard)
        {}

        template<typename HsmType>
        void doTransition(HsmType* hsm)
        {
            // Evaluate guard if it exists
            bool result = this->guard && CALL_MEMBER_FN(hsm, this->guard);
            if (!(this->guard) || result) {
                if (this->action) {
                    CALL_MEMBER_FN(hsm, this->action);
                }
            }
        }
    };

    using StateEventPair = std::pair<State&, Event>;
    struct HashStateEventPair
    {
        size_t operator()(const StateEventPair& s) const
        {
            auto a = s.first.id;
            auto b = s.second.id;
            return (a + b) * (a + b + 1) / 2 + a;
        }
    };

    using TransitionTableElement = std::pair<StateEventPair, Transition>;
    using TransitionTable =
      std::unordered_map<typename TransitionTableElement::first_type,
                         typename TransitionTableElement::second_type,
                         HashStateEventPair>;

  public:
    Transition* next(State& fromState, Event const& onEvent)
    {
        // Check if event in Hsm
        StateEventPair pair(fromState, onEvent);
        auto it = data_.find(pair);
        if (it != data_.end()) {
            return &it->second;
        }

        return nullptr;
    }

    void print()
    {
        for (const auto& it : *this) {
            LOG(INFO) << it.first.first->name << "," << it.first.second.id
                      << ":" << it.second->toState.name << "\n";
        }
    }

    void add(State& fromState,
             Event const& onEvent,
             State& toState,
             ActionFn action = nullptr,
             GuardFn guard = nullptr)
    {
        Transition t(toState, action, guard);
        addTransition(fromState, onEvent, t);
        eventSet_.insert(onEvent);
    }

    std::set<Event> const& getEvents() const { return eventSet_; }

  private:
    void addTransition(State& fromState,
                       Event const& onEvent,
                       Transition const& t)
    {
        StateEventPair pair(fromState, onEvent);
        TransitionTableElement e(pair, t);
        data_.insert(e);
    }
    TransitionTable data_;
    std::set<Event> eventSet_;
};

} // namespace tsm
