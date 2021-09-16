#pragma once

#include "Event.h"
#include "State.h"
#include "Transition.h"

namespace tsm {
///
/// Interface for any Hierarchical State Machine.
///
struct IHsm : public State
{
    explicit IHsm(IHsm* parent = nullptr)
      : State()
      , parent_(parent)
    {}

    void startSM() { this->onEntry(tsm::null_event); }
    void stopSM() { this->onExit(tsm::null_event); }
    void onEntry(Event const& e) override
    {
        currentState_ = this->getStartState();
        currentState_->onEntry(e);

        if (parent_ != nullptr) {
            parent_->setCurrentHsm(this);
        }
    }

    void onExit(Event const& e) override
    {
        // TODO (sriram): Does the sub-Hsm remember which state it was in at
        // exit? This really depends on exit/history policy. Sometimes you
        // want to retain state information when you exit a sub-Hsm for
        // certain events. Maybe adding a currenEvent_ variable would allow
        // HsmDefs to override onExit appropriately. Currently as you see,
        // the policy is to 'forget' on exit by setting the currentState_ to
        // nullptr.
        if (currentState_ != nullptr) {
            currentState_->onExit(e);
            currentState_ = nullptr;
        }
        if (parent_ != nullptr) {
            parent_->setCurrentHsm(nullptr);
        }
    }

    IHsm* getCurrentHsm() { return currentHsm_; }
    void setCurrentHsm(IHsm* currentHsm) { currentHsm_ = currentHsm; }

    void dispatch(Event const& e)
    {
        if (currentHsm_ != nullptr) {
            currentHsm_->dispatch(e);
        } else {
            this->handle(e);
        }
    }

    virtual void handle(Event const&) = 0;

    IHsm* getParent() const { return parent_; }
    void setParent(IHsm* parent) { parent_ = parent; }

    virtual State* getCurrentState() { return currentState_; }
    void setCurrentState(State* s) { currentState_ = s; }

    virtual State* getStartState() { return startState_; }
    void setStartState(State* s) { startState_ = s; }

    State* getStopState() { return stopState_; }
    void setStopState(State* s) { stopState_ = s; }

  private:
    IHsm* parent_;
    IHsm* currentHsm_{};

  protected:
    State* currentState_{};
    State* startState_{};
    State* stopState_{};
};

///
/// Implements a Hierarchical State Machine.
///
template<typename HsmDef>
struct Hsm : public IHsm
{
    using StateTransitionTable = StateTransitionTableT<HsmDef>;
    using Transition = typename StateTransitionTableT<HsmDef>::Transition;
    using ActionFn = void (HsmDef::*)();
    using GuardFn = bool (HsmDef::*)();

    explicit Hsm(IHsm* parent = nullptr)
      : IHsm(parent)
    {}

    void handle(Event const& nextEvent) override
    {
        LOG(INFO) << "Current State:" << this->currentState_->id
                   << " Event:" << nextEvent.id;

        Transition* t = this->next(*this->currentState_, nextEvent);

        if (!t) {
            // If transition does not exist, pass event to parent Hsm
            if (this->getParent() != nullptr) {
                // TODO(sriram) : should call onExit? UML spec *seems* to say
                // yes! invoking onExit() here will not work for Orthogonal
                // state machines this->onExit(nextEvent);
                this->getParent()->handle(nextEvent);
            } else {
                LOG(ERROR) << "Reached top level Hsm. Cannot handle event";
            }
        } else {

            // Perform entry and exit actions in the doTransition function.
            // If just an internal transition, Entry and exit actions are
            // not performed
            t->doTransition(static_cast<HsmDef*>(this), nextEvent);

            if (this->currentState_ == this->getStopState()) {
                // LOG(INFO) << this->id << " Reached stop state. Exiting.";
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

    StateTransitionTable& getTable() const { return table_; }
    std::set<Event> const& getEvents() const { return table_.getEvents(); }

  protected:
    StateTransitionTable table_;
};
} // namespace tsm
