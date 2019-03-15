#pragma once

#include "Event.h"
#include "State.h"
#include "HsmExecutor.h"

#include <memory>
namespace tsm {
template<typename HsmDef1, typename HsmDef2>
struct OrthogonalHsmExecutor : public IHsmDef
{
    using type = OrthogonalHsmExecutor<HsmDef1, HsmDef2>;
    using SM1Type = HsmExecutor<HsmDef1>;
    using SM2Type = HsmExecutor<HsmDef2>;

    OrthogonalHsmExecutor(std::string const& name, IHsmDef* parent = nullptr)
      : IHsmDef(name, parent)
      , hsm1_(SM1Type(this))
      , hsm2_(SM2Type(this))
    {}

    void startSM() { onEntry(tsm::null_event); }

    void onEntry(Event const& e) override
    {
        DLOG(INFO) << "Entering: " << this->name;
        this->currentState_ = &hsm1_;
        hsm1_.onEntry(e);
        hsm2_.onEntry(e);
    }

    void stopSM() { onExit(tsm::null_event); }

    void onExit(Event const& e) override
    {
        // TODO(sriram): hsm1->currentState_ = nullptr; etc.

        // Stopping a Hsm means stopping all of its sub Hsms
        hsm1_.onExit(e);
        hsm2_.onExit(e);
    }

    void execute(Event const& nextEvent) override
    {
        if (hsm1_.getEvents().find(nextEvent) != hsm1_.getEvents().end()) {
            this->currentState_ = &hsm1_;
            hsm1_.execute(nextEvent);
        } else if (hsm2_.getEvents().find(nextEvent) !=
                   hsm2_.getEvents().end()) {
            this->currentState_ = &hsm2_;
            hsm2_.execute(nextEvent);
        } else {
            if (parent_) {
                parent_->execute(nextEvent);
            } else {
                LOG(ERROR) << "Reached top level Hsm. Cannot handle event";
            }
        }
    }

    IHsmDef* dispatch(IHsmDef*) override
    {
        SM1Type* hsm = dynamic_cast<SM1Type*>(this->currentState_);
        if (hsm) {
            return hsm1_.dispatch(&hsm1_);
        } else {
            return hsm2_.dispatch(&hsm2_);
        }
    }

    State* getStartState() override { return &hsm1_; }
    State* getStopState() override { return nullptr; }

    SM1Type& getHsm1() { return hsm1_; }
    SM2Type& getHsm2() { return hsm2_; }

    SM1Type hsm1_;
    SM2Type hsm2_;
};

} // namespace tsm
