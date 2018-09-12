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
#include <unordered_map>

using std::shared_ptr;

namespace tsm {

typedef std::pair<std::shared_ptr<State>, Event> StateEventPair;

template<typename DerivedHSM>
struct StateMachine : public State
{
    using ActionFn = void (DerivedHSM::*)(void);
    using GuardFn = bool (DerivedHSM::*)(void);

    typedef TransitionT<State, Event, ActionFn, GuardFn> Transition;

    typedef std::unordered_map<StateEventPair, shared_ptr<Transition>>
      TransitionTable;

    struct StateTransitionTable : private TransitionTable
    {
        using TransitionTable::end;
        using TransitionTable::find;
        using TransitionTable::insert;

      public:
        shared_ptr<Transition> next(shared_ptr<State> fromState, Event onEvent)
        {
            // Check if event in HSM
            StateEventPair pair(fromState, onEvent);
            auto it = find(pair);
            if (it != end()) {
                return it->second;
            }

            // print();
            std::ostringstream s;
            s << "No Transition:" << fromState->name
              << "\tonEvent:" << onEvent.id;
            LOG(ERROR) << s.str();
            return nullptr;
        }

        void print()
        {
            for (const auto& it : *this) {
                LOG(INFO) << it.first.first->name << "," << it.first.second.id
                          << ":" << it.second->toState->name << "\n";
            }
        }

        size_t size() { return TransitionTable::size(); }
    };

    typedef typename ::std::pair<StateEventPair, shared_ptr<Transition>>
      TransitionTableElement;

  public:
    StateMachine() = delete;

    StateMachine(std::string name,
                 shared_ptr<State> startState,
                 shared_ptr<State> stopState,
                 EventQueue<Event>& eventQueue,
                 State* parent = nullptr)
      : State(name)
      , interrupt_(false)
      , currentState_(nullptr)
      , startState_(startState)
      , stopState_(std::move(stopState))
      , eventQueue_(eventQueue)
      , parent_(parent)
    {}

    StateMachine(std::string name,
                 EventQueue<Event>& eventQueue,
                 State* parent = nullptr)
      : StateMachine(name, nullptr, nullptr, eventQueue, parent)
    {}

    virtual ~StateMachine() override = default;

    void add(shared_ptr<State> fromState,
             Event onEvent,
             shared_ptr<State> toState,
             ActionFn action = nullptr,
             GuardFn guard = nullptr)
    {

        shared_ptr<Transition> t = std::make_shared<Transition>(
          fromState, onEvent, toState, action, guard);
        addTransition(fromState, onEvent, t);
    }

    virtual shared_ptr<State> getStartState() const { return startState_; }

    virtual shared_ptr<State> getStopState() const { return stopState_; }

    void OnEntry() override
    {
        DLOG(INFO) << "Entering: " << this->name;
        startHSM();
    }

    void OnExit() override
    {
        currentState_ = nullptr;
        // Stopping a HSM means stopping all of its sub HSMs
        stopHSM();
        LOG(INFO) << "Exiting: " << name;
    }

    auto& getTable() const { return table_; }

    void startHSM()
    {
        LOG(INFO) << "starting: " << name;
        currentState_ = getStartState();
        // Only start a separate thread if you are the base Hsm
        // Potential issue: An HSM with parent improperly set to null
        if (!parent_) {
            smThread_ = std::thread(&StateMachine::execute, this);
        }
        LOG(INFO) << "started: " << name;
    }

    void stopHSM()
    {
        LOG(INFO) << "stopping: " << name;

        interrupt_ = true;

        if (!parent_) {
            eventQueue_.stop();
            smThread_.join();
        }
        LOG(INFO) << "stopped: " << name;
    }

    void execute() override
    {
        while (!interrupt_) {
            if (currentState_ == getStopState()) {
                LOG(INFO) << this->name << " Done Exiting... ";
                interrupt_ = true;
                break;
            } else {
                LOG(INFO) << this->name << ": Waiting for event";

                Event nextEvent;
                try {
                    nextEvent = eventQueue_.nextEvent();
                } catch (EventQueueInterruptedException const& e) {
                    if (!interrupt_) {
                        throw e;
                    }
                    LOG(WARNING)
                      << this->name << ": Exiting event loop on interrupt";
                    break;
                }

                LOG(INFO) << "Current State:" << currentState_->name
                          << " Event:" << nextEvent.id;

                shared_ptr<Transition> t =
                  table_.next(currentState_, nextEvent);

                if (!t) {
                    if (parent_) {
                        eventQueue_.addFront(nextEvent);
                        break;
                    } else {
                        LOG(ERROR)
                          << "Reached top level HSM. Cannot handle event";
                        continue;
                    }
                }

                bool result =
                  t->guard && (static_cast<DerivedHSM*>(this)->*(t->guard))();

                ActionFn action = nullptr;
                if (!(t->guard) || result) {
                    action = t->template doTransition<DerivedHSM>();
                    currentState_ = t->toState;

                    LOG(INFO) << "Next State:" << currentState_->name;
                    if (action) {
                        (static_cast<DerivedHSM*>(this)->*action)();
                    }

                    // Now execute the current state
                    currentState_->execute();
                } else {
                    LOG(INFO) << "Guard prevented transition";
                }
            }
        }
    }

    virtual shared_ptr<State> const& getCurrentState() const
    {
        LOG(INFO) << "GetState : " << this->name;
        return currentState_;
    }

    State* getParent() const { return parent_; }

  protected:
    std::atomic<bool> interrupt_;
    shared_ptr<State> currentState_;
    shared_ptr<State> startState_;
    shared_ptr<State> stopState_;
    EventQueue<Event>& eventQueue_;
    StateTransitionTable table_;
    State* parent_;
    std::thread smThread_;

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

// Provide a hash function for StateEventPair
namespace std {
template<>
struct hash<tsm::StateEventPair>
{
    size_t operator()(const tsm::StateEventPair& s) const
    {
        shared_ptr<tsm::State> statePtr = s.first;
        tsm::Event event = s.second;
        size_t address = reinterpret_cast<size_t>(statePtr.get());
        size_t id = event.id;
        size_t hash_value = hash<int>{}(address) ^ (hash<size_t>{}(id) << 1);
        return hash_value;
    }
};

} // namespace std
