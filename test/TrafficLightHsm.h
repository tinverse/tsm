#include "tsm.h"

using tsm::Event;
using tsm::EventQueue;
using tsm::Hsm;
using tsm::State;

namespace tsmtest {
struct TrafficLightHsm : public Hsm<TrafficLightHsm>
{
    static const int G2WALK = 30;
    struct LightState : public State
    {
        friend TrafficLightHsm;
        LightState(std::string const& name,
                   uint64_t limit,
                   TrafficLightHsm* parent)
          : State(name)
          , limit_(limit)
          , parent_(parent)
        {}

        virtual ~LightState() = default;

        // void onEntry(Event const&) override { parent_->ticks_ = 0; }
        void execute(Event const&) override { ++parent_->ticks_; }
        // void onExit(Event const&) override { std::cout << "Exiting:" <<
        // this->name << " ticks:" << parent_->ticks_ << std::endl; }
        uint64_t getLimit() const { return limit_; }

      private:
        const uint64_t limit_;
        TrafficLightHsm* parent_;
    };

    bool guard()
    {
        auto* state = dynamic_cast<LightState*>(this->currentState_);
        bool ret = (this->ticks_ > state->limit_);
        if (state->id == G2.id) {
            ret |= (this->ticks_ > G2WALK && walkPressed);
        }
        return ret;
    }

    void action()
    {
        auto* state = dynamic_cast<LightState*>(this->currentState_);
        // disable walkPressed when exiting G2
        if (state->id == G2.id && walkPressed) {
            walkPressed = false;
        }
        this->ticks_ = 0;
    }

    ActionFn actionfn = &TrafficLightHsm::action;
    GuardFn guardfn = &TrafficLightHsm::guard;

    TrafficLightHsm()
      : Hsm<TrafficLightHsm>("Traffic Light Hsm")
      , G1("G1", 30, this)
      , Y1("Y1", 5, this)
      , G2("G2", 60, this)
      , Y2("Y2", 5, this)
      , walkPressed(false)
      , ticks_(0)
    {
        // TransitionTable. A table is not needed
        // for this class of problem. One function
        // with a switch..case can replace the action,
        // guard and transition functions.
        add(G1, timer_event, Y1, actionfn, guardfn);
        add(Y1, timer_event, G2, actionfn, guardfn);
        add(G2, timer_event, Y2, actionfn, guardfn);
        add(Y2, timer_event, G1, actionfn, guardfn);
    }

    void pressWalk() { walkPressed = true; }

    virtual ~TrafficLightHsm() = default;

    State* getStartState() override { return &G1; }
    State* getStopState() override { return nullptr; }

    // States
    LightState G1;
    LightState Y1;
    LightState G2;
    LightState Y2;

    // Events
    Event timer_event;

    // Walk button
    bool walkPressed;
    uint64_t ticks_;
};
} // namespace tsmtest
