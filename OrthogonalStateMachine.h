#pragma once

#include "Event.h"
#include "State.h"
#include "StateMachine.h"

namespace tsm {
template<typename HSMDef1, typename HSMDef2>
struct OrthogonalStateMachine : public State
{
    using type = OrthogonalStateMachine<HSMDef1, HSMDef2>;
    using SM1Type = HSMBehavior<HSMDef1>;
    using SM2Type = HSMBehavior<HSMDef2>;

    OrthogonalStateMachine(std::string name, State* parent = nullptr)
      : State(name)
      , hsm1_(std::make_shared<SM1Type>(this))
      , hsm2_(std::make_shared<SM2Type>(this))
      , parent_(parent)
      , currentState_(nullptr)
    {}

    void startSM() { onEntry(Event::dummy_event); }

    void onEntry(Event const& e) override
    {
        DLOG(INFO) << "Entering: " << this->name;
        currentState_ = hsm1_;
        hsm1_->onEntry(e);
        hsm2_->onEntry(e);
    }

    void stopSM() { onExit(Event::dummy_event); }

    void onExit(Event const& e) override
    {
        // TODO(sriram): hsm1->currentState_ = nullptr; etc.

        // Stopping a HSM means stopping all of its sub HSMs
        hsm1_->onExit(e);
        hsm2_->onExit(e);
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
