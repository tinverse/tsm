#pragma once
#ifndef CALL_MEMBER_FN
#define CALL_MEMBER_FN(object, ptrToMember) ((object)->*(ptrToMember))()
#endif
#include "Event.h"
#include "State.h"

#include <set>
#include <unordered_map>
namespace tsm {

template<typename FsmDef>
struct StateTransitionTableT
{
    using ActionFn = void (FsmDef::*)();
    using GuardFn = bool (FsmDef::*)();

    struct Transition
    {
        Transition(State& fromState,
                   Event const& event,
                   State& toState,
                   ActionFn action,
                   GuardFn guard)
          : fromState(fromState)
          , onEvent(event)
          , toState(toState)
          , action(action)
          , guard(guard)
        {}

        virtual ~Transition() = default;

        bool doTransition(FsmDef* hsm)
        {
            bool transitioned = false;
            if (!hsm) {
                // throw NullPointerException;
            }

            // Evaluate guard if it exists
            bool result = guard && CALL_MEMBER_FN(hsm, guard);

            if (!guard || result) {
                // Perform entry and exit actions in the doTransition function.
                // If just an internal transition, Entry and exit actions are
                // not performed

                this->fromState.onExit(onEvent);
                if (action) {
                    CALL_MEMBER_FN(hsm, action);
                }
                this->toState.onEntry(onEvent);
                transitioned = true;
            } else {
                DLOG(INFO) << "Guard prevented transition";
            }
            return transitioned;
        }
        State& fromState;
        Event onEvent;
        State& toState;
        ActionFn action;
        GuardFn guard;
    };

    struct InternalTransition : public Transition
    {
        InternalTransition(State& fromState,
                           Event const& event,
                           ActionFn action,
                           GuardFn guard)
          : Transition(fromState, event, fromState, action, guard)
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
            } else {
                DLOG(INFO) << "Guard prevented transition";
            }
        }
    };

    using StateEventPair = std::pair<State&, Event>;
    struct HashStateEventPair
    {
        size_t operator()(const StateEventPair& s) const
        {
            State* statePtr = &s.first;
            uint64_t id_s = statePtr->id;
            tsm::Event event = s.second;
            auto address = reinterpret_cast<uintptr_t>(statePtr);
            uint64_t id_e = event.id;
            size_t hash_value = std::hash<uint64_t>{}(id_s) ^
                                std::hash<uintptr_t>{}(address) ^
                                (std::hash<uint64_t>{}(id_e) << 1);
            return hash_value;
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
