#pragma once

#include "Event.h"
#include "State.h"
#include "Transition.h"

namespace tsm {
///
/// Interface for any Hierarchical SM.
///
struct IHsm
{
    IHsm() = delete;

    explicit IHsm(IHsm* parent)
      : parent_(parent)
      , currentHsm_(nullptr)
    {}

    IHsm(IHsm const&) = delete;
    IHsm(IHsm&&) = delete;

    virtual ~IHsm() = default;

    IHsm* getCurrentHsm() { return currentHsm_; }
    void setCurrentHsm(IHsm* currentHsm) { currentHsm_ = currentHsm; }

    // The Hsm has some level of freedom to pick a state so an event
    // can be dispatched to it. This provides a level of indirection
    // which could be used to our advantage. For e.g. take a look at
    // how OrthogonalHsm and Hsm override this method.
    virtual State* dispatch() = 0;

    IHsm* getParent() const { return parent_; }
    void setParent(IHsm* parent) { parent_ = parent; }

  protected:
    IHsm* parent_;
    IHsm* currentHsm_;
};

///
/// Supports Mealy and Moore machines
///
template<typename HsmDef>
struct Hsm
  : public IHsm
  , public State
{
    using StateTransitionTable = StateTransitionTableT<HsmDef>;
    using Transition = typename StateTransitionTableT<HsmDef>::Transition;
    using ActionFn = void (HsmDef::*)();
    using GuardFn = bool (HsmDef::*)();

    Hsm(std::string const& name)
      : IHsm(nullptr)
      , State(name)
      , currentState_(nullptr)
    {}

    Hsm(std::string const& name, IHsm* parent)
      : IHsm(parent)
      , State(name)
      , currentState_(nullptr)
    {}

    Hsm(IHsm const&) = delete;
    Hsm(IHsm&&) = delete;

    virtual ~Hsm() { stopSM(); }
    void startSM() { this->onEntry(tsm::null_event); }
    void stopSM() { this->onExit(tsm::null_event); }
    void onEntry(Event const& e) override
    {
        DLOG(INFO) << "Entering: " << this->name;
        this->currentState_ = this->getStartState();

        if (this->parent_) {
            this->parent_->setCurrentHsm(this);
        }

        this->currentState_->execute(e);
    }

    void onExit(Event const&) override
    {
        // TODO (sriram): Does the sub-Hsm remember which state it was in at
        // exit? This really depends on exit/history policy. Sometimes you
        // want to retain state information when you exit a sub-Hsm for
        // certain events. Maybe adding a currenEvent_ variable would allow
        // HsmDefs to override onExit appropriately. Currently as you see,
        // the policy is to 'forget' on exit by setting the currentState_ to
        // nullptr.
        DLOG(INFO) << "Exiting: " << this->name;
        this->currentState_ = nullptr;

        if (this->parent_) {
            this->parent_->setCurrentHsm(nullptr);
        }
    }

    // Here is an event. Give me the state that has a transition corresponding
    // to it.
    State* dispatch() override
    {
        IHsm* currentHsm = this->getCurrentHsm();
        if (currentHsm != nullptr) {
            return currentHsm->dispatch();
        }
        return this;
    }

    void execute(Event const& nextEvent) override
    {
        DLOG(INFO) << "Current State:" << this->currentState_->name
                   << " Event:" << nextEvent.id;

        Transition* t = this->next(*this->currentState_, nextEvent);

        if (!t) {
            // If transition does not exist, pass event to parent Hsm
            if (this->parent_) {
                // TODO(sriram) : should call onExit? UML spec *seems* to say
                // yes! invoking onExit() here will not work for Orthogonal
                // state machines this->onExit(nextEvent);
                auto* parent = dynamic_cast<State*>(this->parent_);
                parent->execute(nextEvent);
            } else {
                DLOG(ERROR) << "Reached top level Hsm. Cannot handle event";
            }
        } else {
            // This call to the Simple state's execute method makes it
            // behave like a moore machine.
            bool isAtomicState =
              (dynamic_cast<IHsm*>(this->currentState_) == nullptr);
            if (isAtomicState) {
                this->currentState_->execute(nextEvent);
            }

            // Perform entry and exit actions in the doTransition function.
            // If just an internal transition, Entry and exit actions are
            // not performed
            if (t->doTransition(static_cast<HsmDef*>(this))) {
                this->currentState_ = &t->toState;
                IHsm* currentHsm = dynamic_cast<IHsm*>(this->currentState_);
                // DLOG(INFO) << "Next State:" << this->currentState_->name;
                if (currentHsm != nullptr) {
                    setCurrentHsm(currentHsm);
                }
            }

            if (this->currentState_ == this->getStopState()) {
                // DLOG(INFO) << this->name << " Reached stop state. Exiting...
                // ";
                this->onExit(tsm::null_event);
            }
        }
    }

    void add(State& fromState,
             Event const& onEvent,
             State& toState,
             ActionFn action = nullptr,
             GuardFn guard = nullptr)
    {
        table_.add(fromState, onEvent, toState, action, guard);
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
    std::set<Event> const& getEvents() const { return table_.getEvents(); }

    virtual State* getStartState() = 0;
    virtual State* getStopState() = 0;

  protected:
    StateTransitionTable table_;
    State* currentState_;
};
} // namespace tsm
