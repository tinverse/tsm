#pragma once

#include "Event.h"
#include "State.h"
#include "StateMachine.h"

#include <memory>
namespace tsm {
template<typename HSMDef1, typename HSMDef2>
struct OrthogonalStateMachine : public State
{
    using type = OrthogonalStateMachine<HSMDef1, HSMDef2>;
    using SM1Type = StateMachine<HSMDef1>;
    using SM2Type = StateMachine<HSMDef2>;

    OrthogonalStateMachine(std::string name, State* parent = nullptr)
      : State(name)
      , hsm1_(SM1Type(this))
      , hsm2_(SM2Type(this))
      , parent_(parent)
      , currentState_(nullptr)
    {}

    void startSM() { onEntry(Event::dummy_event); }

    void onEntry(Event const& e) override
    {
        DLOG(INFO) << "Entering: " << this->name;
        currentState_ = &hsm1_;
        hsm1_.onEntry(e);
        hsm2_.onEntry(e);
    }

    void stopSM() { onExit(Event::dummy_event); }

    void onExit(Event const& e) override
    {
        // TODO(sriram): hsm1->currentState_ = nullptr; etc.

        // Stopping a HSM means stopping all of its sub HSMs
        hsm1_.onExit(e);
        hsm2_.onExit(e);
    }

    void execute(Event const& nextEvent) override
    {
        if (hsm1_.getEvents().find(nextEvent) != hsm1_.getEvents().end()) {
            currentState_ = &hsm1_;
            hsm1_.execute(nextEvent);
        } else if (hsm2_.getEvents().find(nextEvent) !=
                   hsm2_.getEvents().end()) {
            currentState_ = &hsm2_;
            hsm2_.execute(nextEvent);
        } else {
            if (parent_) {
                parent_->execute(nextEvent);
            } else {
                LOG(ERROR) << "Reached top level HSM. Cannot handle event";
            }
        }
    }

    State* getCurrentState() override { return &hsm1_; }

    State* dispatch(State*)
    {
        SM1Type* hsm = dynamic_cast<SM1Type*>(currentState_);
        if (hsm) {
            return hsm1_.dispatch(&hsm1_);
        } else {
            return hsm2_.dispatch(&hsm2_);
        }
    }

    auto& getHsm1() { return hsm1_; }
    auto& getHsm2() { return hsm2_; }

    SM1Type hsm1_;
    SM2Type hsm2_;
    State* parent_;
    State* currentState_;
};

} // namespace tsm
