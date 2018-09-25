#pragma once

#include "Event.h"
#include "State.h"
#include "StateMachineDef.h"

namespace tsm {
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

} // namespace tsm
