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
    IHsm() = delete;

    explicit IHsm(std::string const& name, IHsm* parent)
      : State(name)
      , parent_(parent)
      , currentHsm_(nullptr)
      , currentState_(nullptr)
      , startState_(nullptr)
      , stopState_(nullptr)
    {}

    IHsm(IHsm const&) = delete;
    IHsm(IHsm&&) = delete;

    ~IHsm() override = default;

    void startSM() { this->onEntry(tsm::null_event); }
    void stopSM() { this->onExit(tsm::null_event); }
    void onEntry(Event const&) override
    {
        currentState_ = this->getStartState();

        if (parent_ != nullptr) {
            parent_->setCurrentHsm(this);
        }
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
        currentState_ = nullptr;
        if (parent_ != nullptr) {
            parent_->setCurrentHsm(nullptr);
        }
        startState_ = nullptr;
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

    State* getCurrentState() { return currentState_; }


    State* getStartState() { return startState_; }
    void setStartState(State* s) { startState_ = s; }

    State* getStopState() { return stopState_; }
    void setStopState(State* s) { stopState_ = s; }

  private:
    IHsm* parent_;
    IHsm* currentHsm_;
  protected:
    State* currentState_;
    State* startState_;
    State* stopState_;
};

///
/// Implements a Hierarchical State Machine.
///
template<typename HsmDef>
struct Hsm
  : public IHsm
{
    using StateTransitionTable = StateTransitionTableT<HsmDef>;
    using Transition = typename StateTransitionTableT<HsmDef>::Transition;
    using ActionFn = void (HsmDef::*)();
    using GuardFn = bool (HsmDef::*)();

    Hsm(std::string const& name)
      : IHsm(name, nullptr)
    {}

    Hsm(std::string const& name, IHsm* parent)
      : IHsm(name, parent)
    {}

    Hsm(Hsm const&) = delete;
    Hsm(Hsm&&) = delete;

    ~Hsm() override { IHsm::stopSM(); }

    void handle(Event const& nextEvent) override
    {
        DLOG(INFO) << "Current State:" << this->currentState_->name
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
                DLOG(ERROR) << "Reached top level Hsm. Cannot handle event";
            }
        } else {

            // Perform entry and exit actions in the doTransition function.
            // If just an internal transition, Entry and exit actions are
            // not performed
            if (t->doTransition(static_cast<HsmDef*>(this))) {
                this->currentState_ = &t->toState;

                auto* currentHsm = dynamic_cast<IHsm*>(this->currentState_);
                // DLOG(INFO) << "Next State:" << this->currentState_->name;
                if (currentHsm != nullptr) {
                    setCurrentHsm(currentHsm);
                }
            }

            if (this->currentState_ == this->getStopState()) {
                // DLOG(INFO) << this->name << " Reached stop state. Exiting.";
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

template<typename HsmDef>
struct MooreHsm
  : public IHsm
{
    MooreHsm(std::string const& name)
      : IHsm(name, nullptr)
    {}

    MooreHsm(std::string const& name, IHsm* parent)
      : IHsm(name, parent)
    {}

    MooreHsm(MooreHsm const&) = delete;
    MooreHsm(MooreHsm&&) = delete;

    ~MooreHsm() override { IHsm::stopSM(); }

    void handle(Event const& nextEvent) override
    {
        currentState_ = currentState_->execute(nextEvent);
        if (currentState_ == this->getStopState()) {
            this->onExit(tsm::null_event);
        }
    }
};

} // namespace tsm
