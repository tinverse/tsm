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
        shared_ptr<Transition> next(shared_ptr<State> fromState, Event onEvent)
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
                    shared_ptr<State> startState,
                    shared_ptr<State> stopState,
                    State* parent = nullptr)
      : State(name)
      , startState_(startState)
      , stopState_(std::move(stopState))
      , parent_(parent)
    {}

    StateMachineDef(std::string const& name, State* parent = nullptr)
      : StateMachineDef(name, nullptr, nullptr, parent)
    {}

    void add(shared_ptr<State> fromState,
             Event onEvent,
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

    auto& getTable() const { return table_; }
    auto& getEvents() const { return eventSet_; }

    virtual shared_ptr<State> getStartState() const { return startState_; }
    virtual shared_ptr<State> getStopState() const { return stopState_; }

    State* getParent() const override { return parent_; }
    void setParent(State* parent) { parent_ = parent; }

  protected:
    StateTransitionTable table_;
    shared_ptr<State> startState_;
    shared_ptr<State> stopState_;
    State* parent_;
    std::set<Event> eventSet_;

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

template<typename HSMDef>
struct StateMachine : public HSMDef
{
    using Transition = typename StateMachineDef<HSMDef>::Transition;

    StateMachine(State* parent = nullptr)
      : HSMDef(parent)
      , currentState_(nullptr)
    {}

    virtual ~StateMachine() = default;

    void onEntry() override
    {
        DLOG(INFO) << "Entering: " << this->name;
        currentState_ = this->getStartState();
    }

    void onExit() override
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

    // traverse the hsm hierarchy down.
    State* dispatch(State* state) const
    {
        State* parent = state;
        State* kid = parent->getCurrentState().get();
        while (kid->getParent()) {
            parent = kid;
            kid = kid->getCurrentState().get();
        }
        return parent;
    }

    void execute(Event const& nextEvent) override
    {

        LOG(INFO) << "Current State:" << currentState_->name
                  << " Event:" << nextEvent.id;

        HSMDef* thisDef = static_cast<HSMDef*>(this);
        shared_ptr<Transition> t = thisDef->next(currentState_, nextEvent);

        if (!t) {
            // If transition does not exist, pass event to parent HSM
            if (this->parent_) {
                // TODO(sriram) : should call onExit? UML spec seems to say yes
                // invoking onExit() here will not work for Orthogonal state
                // machines
                this->parent_->execute(nextEvent);
            } else {
                LOG(ERROR) << "Reached top level HSM. Cannot handle event";
            }
        } else {
            shared_ptr<State> nextState = nullptr;

            // Evaluate guard if it exists
            bool result = t->guard && (thisDef->*(t->guard))();

            if (!(t->guard) || result) {
                // Perform entry and exit actions in the doTransition function.
                // If just an internal transition, Entry and exit actions are
                // not performed
                t->template doTransition<HSMDef>(thisDef);

                currentState_ = t->toState;

                DLOG(INFO) << "Next State:" << currentState_->name;
            } else {
                LOG(INFO) << "Guard prevented transition";
            }
            if (currentState_ == this->getStopState()) {
                DLOG(INFO) << this->name << " Done Exiting... ";
                onExit();
            }
        }
    }

    shared_ptr<State> const getCurrentState() const override
    {
        DLOG(INFO) << "GetState : " << this->name;
        return currentState_;
    }

  protected:
    shared_ptr<State> currentState_;
};

template<typename StateType>
struct StateMachineExecutionPolicy
{
    StateMachineExecutionPolicy() = delete;
    StateMachineExecutionPolicy(StateType* sm)
      : sm_(sm)
    {}

    virtual ~StateMachineExecutionPolicy() = default;

    virtual void start() = 0;
    virtual void stop() = 0;

  protected:
    StateType* sm_;
};

template<typename StateType>
struct SeparateThreadExecutionPolicy
  : public StateMachineExecutionPolicy<StateType>
{
    using EventQueue = EventQueueT<Event, std::mutex>;
    using ThreadCallback = void (SeparateThreadExecutionPolicy::*)();

    SeparateThreadExecutionPolicy() = delete;

    SeparateThreadExecutionPolicy(StateType* sm)
      : StateMachineExecutionPolicy<StateType>(sm)
      , threadCallback_(&SeparateThreadExecutionPolicy::step)
      , interrupt_(false)
    {}

    void start() override { smThread_ = std::thread(threadCallback_, this); }

    void stop() override
    {
        interrupt_ = true;
        eventQueue_.stop();
        smThread_.join();
    }

    void step()
    {
        while (!interrupt_) {
            Event nextEvent;
            try {
                // This is a blocking wait
                nextEvent = eventQueue_.nextEvent();
            } catch (EventQueueInterruptedException const& e) {
                if (!interrupt_) {
                    throw e;
                }
                LOG(WARNING)
                  << this->sm_->name << ": Exiting event loop on interrupt";
                return;
            }
            // go down the HSM hierarchy to handle the event as that is the
            // "most active state"
            // this->sm_->dispatch(this->sm_)->execute(nextEvent);
            this->sm_->dispatch(this->sm_)->execute(nextEvent);
        }
    };

    void sendEvent(Event event) { eventQueue_.addEvent(event); }

  protected:
    ThreadCallback threadCallback_;
    std::thread smThread_;
    EventQueue eventQueue_;

    // R/W is atomic for bool
    bool interrupt_;
};

template<typename StateType>
struct ParentThreadExecutionPolicy
  : public StateMachineExecutionPolicy<StateType>
{
    using EventQueue = EventQueueT<Event, dummy_mutex>;

    ParentThreadExecutionPolicy() = delete;

    ParentThreadExecutionPolicy(StateType* sm)
      : StateMachineExecutionPolicy<StateType>(sm)
      , interrupt_(false)
    {}

    void start() override {}

    void stop() override { eventQueue_.stop(); }

    void step()
    {

        if (eventQueue_.empty()) {
            LOG(WARNING) << "Event Queue is empty!";
            return;
        }
        Event nextEvent;
        try {
            // This is a blocking wait
            nextEvent = eventQueue_.nextEvent();
        } catch (EventQueueInterruptedException const& e) {
            if (!interrupt_) {
                throw e;
            }
            LOG(WARNING) << this->sm_->name
                         << ": Exiting event loop on interrupt";
            return;
        }
        // go down the HSM hierarchy to handle the event as that is the
        // "most active state"
        this->sm_->dispatch(this->sm_)->execute(nextEvent);
    }

    void sendEvent(Event event) { eventQueue_.addEvent(event); }

  protected:
    EventQueue eventQueue_;

    // R/W is atomic for bool
    bool interrupt_;
};

template<typename StateType, template<class> class ExecutionPolicy>
struct StateMachineWithExecutionPolicy
  : public StateType
  , public ExecutionPolicy<StateType>
{

    using ExecutionPolicy<StateType>::start;
    using ExecutionPolicy<StateType>::stop;

    StateMachineWithExecutionPolicy()
      : StateType()
      , ExecutionPolicy<StateType>(this)
    {}

    virtual ~StateMachineWithExecutionPolicy() = default;

    void onEntry() override
    {
        StateType::onEntry();
        if (!this->parent_)
            start();
    }

    virtual void onExit() override
    {
        StateType::onExit();
        if (!this->parent_)
            stop();
    }
};

template<typename HSMDef1, typename HSMDef2>
struct OrthogonalStateMachine : public State
{
    using type = OrthogonalStateMachine<HSMDef1, HSMDef2>;
    using SM1Type = StateMachine<HSMDef1>;
    using SM2Type = StateMachine<HSMDef2>;

    OrthogonalStateMachine(std::string name, State* parent = nullptr)
      : State(name)
      , hsm1_(std::make_shared<SM1Type>(this))
      , hsm2_(std::make_shared<SM2Type>(this))
      , parent_(parent)
      , currentState_(nullptr)
    {}

    void onEntry() override
    {
        DLOG(INFO) << "Entering: " << this->name;
        currentState_ = hsm1_;
        hsm1_->onEntry();
        hsm2_->onEntry();
    }

    void onExit() override
    {
        // TODO(sriram): hsm1->currentState_ = nullptr; etc.

        // Stopping a HSM means stopping all of its sub HSMs
        hsm1_->onExit();
        hsm2_->onExit();
    }

    void execute(Event const& nextEvent) override
    {
        if (hsm1_->getEvents().find(nextEvent) != hsm1_->getEvents().end()) {
            currentState_ = hsm1_;
            hsm1_->execute(nextEvent);
        } else if (hsm2_->getEvents().find(nextEvent) !=
                   hsm2_->getEvents().end()) {
            currentState_ = hsm2_;
            hsm2_->execute(nextEvent);
        } else {
            if (parent_) {
                parent_->execute(nextEvent);
            } else {
                LOG(ERROR) << "Reached top level HSM. Cannot handle event";
            }
        }
    }

    shared_ptr<State> const getCurrentState() const override { return hsm1_; }
    State* const dispatch(State*) const
    {

        auto hsm = std::dynamic_pointer_cast<SM1Type>(currentState_);
        if (hsm) {
            return hsm1_->dispatch(hsm1_.get());
        } else {
            return hsm2_->dispatch(hsm2_.get());
        }
    }

    auto& getHsm1() const { return hsm1_; }
    auto& getHsm2() const { return hsm2_; }

    shared_ptr<SM1Type> hsm1_;
    shared_ptr<SM2Type> hsm2_;
    State* parent_;
    shared_ptr<State> currentState_;
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
