#include "tsm.h"

using tsm::Event;
using tsm::IHsm;
using tsm::NamedState;

namespace tsmtest {
struct TrafficLightHsm : public IHsm
{
    static const int G2WALK = 30;
    struct LightState : public NamedState
    {
        LightState(std::string const& name, uint64_t limit, LightState& next)
          : NamedState(name)
          , limit_(limit)
          , next_(next)
        {}

        uint64_t getLimit() const { return limit_; }
        LightState& nextState() { return next_; }

      private:
        const uint64_t limit_;
        LightState& next_;
    };

    TrafficLightHsm()
      : G1("G1", 30, Y1)
      , Y1("Y1", 5, G2)
      , G2("G2", 60, Y2)
      , Y2("Y2", 5, G1)
    {
        this->setStartState(&G1);
    }

    void pressWalk() { walkPressed = true; }

    void handle(Event const& /*unused*/) override
    {
        ++ticks_;
        auto* state = dynamic_cast<LightState*>(this->currentState_);
        bool guard = (this->ticks_ > state->getLimit());
        if (state->id == G2.id) {
            guard |= (walkPressed && (this->ticks_ > G2WALK));
        }

        if (guard) {
            // disable walkPressed when exiting G2
            if ((state->id == G2.id) && walkPressed) {
                walkPressed = false;
            }

            ticks_ = 0;
            // set the next state
            this->currentState_ = &state->nextState();
        }
    }

    // States
    LightState G1, Y1, G2, Y2;

    // Events
    Event timer_event;

    // Walk button
    bool walkPressed{};
    uint64_t ticks_{};
};
} // namespace tsmtest
