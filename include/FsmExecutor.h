#pragma once

#include "Event.h"
#include "FsmDefinition.h"

namespace tsm {
template<typename FsmDef>
struct FsmExecutor : public FsmDef
{
    using Transition = typename FsmDefinition<FsmDef>::Transition;

    FsmExecutor() = default;

    virtual ~FsmExecutor() { stopSM(); }
    void startSM() { this->onEntry(tsm::null_event); }
    void stopSM() { this->onExit(tsm::null_event); }
    void onEntry(Event const& e) override
    {
        DLOG(INFO) << "Entering: " << this->name;
        this->currentState_ = this->getStartState();

        this->currentState_->execute(e);
    }

    void onExit(Event const&) override
    {
        DLOG(INFO) << "Exiting: " << this->name;
        this->currentState_ = nullptr;
    }

    // Here is an event. Give me the state that has a transition corresponding
    // to it.
    FsmDef* dispatch() { return this; }

    void execute(Event const& nextEvent) override
    {
        DLOG(INFO) << "Current State:" << this->currentState_->name
                   << " Event:" << nextEvent.id;

        Transition* t = this->next(*this->currentState_, nextEvent);

        if (!t) {
            DLOG(ERROR) << "No transition from exists";
        } else {
            // This call to the Simple state's execute method makes it
            // behave like a moore machine.
            this->getCurrentState()->execute(nextEvent);

            // If just an internal transition, Entry and exit actions are
            // not performed
            t->template doTransition<FsmDef>(this);

            // Set next state
            this->currentState_ = &t->toState;
            // DLOG(INFO) << "Next State:" << this->currentState_->name;
        }
        if (this->currentState_ == this->getStopState()) {
            // DLOG(INFO) << this->name << " Reached stop state. Exiting...
            // ";
            this->onExit(tsm::null_event);
        }
    }
};
} // namespace tsm
