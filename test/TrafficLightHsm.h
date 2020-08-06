#include "tsm.h"

using tsm::Event;
using tsm::EventQueue;
using tsm::Hsm;
using tsm::MooreHsm;
using tsm::State;

namespace tsmtest {
///
/// The problem is to model traffic lights at a 2-way crossing. The states are
/// G1(30s), Y1(5s), G2(60s), Y2(5s). The signal stays on for the amount of time
/// indicated in the brackets adjacent to the signal before moving on to the
/// next. The added complication is that G2 has a walk signal. If the walk
/// signal is pressed, G2 stays on for only 30s instead of 60s before
/// transitioning to Y2. The trick is to realize that there is only one event
/// for this state machine: The expiry of a timer at say, 1s granularity.
///
struct TrafficLightHsm : public MooreHsm<TrafficLightHsm>
{
    static const int G2WALK = 30;
    struct LightState : public State
    {
        friend TrafficLightHsm;
        LightState(std::string const& name,
                   uint64_t limit,
                   State* nextState,
                   TrafficLightHsm* parent)
          : State(name)
          , limit_(limit)
          , nextState_(nextState)
          , parent_(parent)
        {}

        ~LightState() override
        {
            parent_ = nullptr;
            nextState_ = nullptr;
        }

        State* execute(Event const&) override
        {
            if (++parent_->ticks_ > limit_) {
                this->parent_->ticks_ = 0;
                return nextState_;
            }
            return this;
        }
        uint64_t getLimit() const { return limit_; }
        State* getNextState() const { return nextState_; }

      protected:
        const uint64_t limit_;
        State* nextState_;
        TrafficLightHsm* parent_;
    };

    struct G2 : public LightState
    {
        friend TrafficLightHsm;
        G2(std::string const& name,
                   uint64_t limit,
                   State* nextState,
                   TrafficLightHsm* parent)
          : LightState(name, limit, nextState, parent)
        {}

        ~G2() override = default;

        State* execute(Event const&) override
        {
            if ((this->parent_->walkPressed && (this->parent_->ticks_ > G2_WALK))
                    || (++parent_->ticks_ > limit_)) {
                this->parent_->walkPressed = false;
                this->parent_->ticks_ = 0;
                return this->nextState_;
            }
            return this;
        }
    private:
        const uint64_t G2_WALK = 30;
    };

    TrafficLightHsm()
      : MooreHsm<TrafficLightHsm>("Traffic Light Hsm")
      , g1("G1", 30, &y1, this)
      , y1("Y1", 5, &g2, this)
      , g2("G2", 60, &y2, this)
      , y2("Y2", 5, &g1, this)
      , walkPressed(false)
      , ticks_(0)
    {
        IHsm::setStartState(&g1);
    }

    void pressWalk() { walkPressed = true; }

    ~TrafficLightHsm() override = default;

    // States
    LightState g1;
    LightState y1;
    G2 g2;
    LightState y2;

    // Events
    Event timer_event;

    // Walk button
    bool walkPressed;
    uint64_t ticks_;
};
} // namespace tsmtest
