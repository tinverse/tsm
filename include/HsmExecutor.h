#pragma once

#include "Event.h"
#include "HsmDefinition.h"
#include "State.h"

namespace tsm {
///
/// Supports Mealy and Moore machines
///
template<typename HsmDef>
struct HsmExecutor : public HsmDef
{
    using Transition = typename HsmDefinition<HsmDef>::Transition;

    HsmExecutor(IHsmDef* parent = nullptr)
      : HsmDef(parent)
    {}

    virtual ~HsmExecutor() { stopSM(); }
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
    IHsmDef* dispatch() override
    {
        IHsmDef* currentHsm = this->getCurrentHsm();
        if (currentHsm) {
            return currentHsm->dispatch();
        } else {
            return this;
        }
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
                (this->parent_)->execute(nextEvent);
            } else {
                DLOG(ERROR) << "Reached top level Hsm. Cannot handle event";
            }
        } else {
            // This call to the Simple state's execute method makes it
            // behave like a moore machine.
            if ((dynamic_cast<IHsmDef*>(this->currentState_)) == nullptr) {
                this->currentState_->execute(nextEvent);
            }

            // Perform entry and exit actions in the doTransition function.
            // If just an internal transition, Entry and exit actions are
            // not performed
            t->template doTransition<HsmDef>(this);

            this->currentState_ = &t->toState;
            // DLOG(INFO) << "Next State:" << this->currentState_->name;

            if (this->currentState_ == this->getStopState()) {
                // DLOG(INFO) << this->name << " Reached stop state. Exiting...
                // ";
                this->onExit(tsm::null_event);
            }
        }
    }
};
} // namespace tsm
