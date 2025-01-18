#include "tsm.h"

// Test the HSM using the Catch2 framework
#include <catch2/catch_test_macros.hpp>

using namespace tsm;
using namespace tsm::detail;

// Example usage
// A Switch HSM
struct SwitchHsmContext {
    // Events
    struct Toggle {};

    // States
    struct Off {};
    struct On {};

    using transitions =
      std::tuple<Transition<Off, Toggle, On>, Transition<On, Toggle, Off>>;
};

using SwitchHsm = make_hsm_t<SwitchHsmContext>;

TEST_CASE("SwitchHsm") {
    using SwitchHsm = make_hsm_t<SwitchHsmContext>;
    SwitchHsm hsm;
    REQUIRE(std::holds_alternative<SwitchHsmContext::Off*>(hsm.current_state_));
    hsm.handle(SwitchHsmContext::Toggle());
    REQUIRE(std::holds_alternative<SwitchHsmContext::On*>(hsm.current_state_));
}

struct SwitchHsmContextWithActions {
    struct Off {};
    struct On {};
    struct Toggle {};

    // optional guard or action
    bool canTurnOn() {
        // Decide if we can turn on...
        return true;
    }
    void onTurnOn() { on_count_++; }

    // Static wrappers that forward to the instance
    static bool canTurnOnGuard(SwitchHsmContextWithActions& ctx) {
        return ctx.canTurnOn();
    }
    static void onTurnOnAction(SwitchHsmContextWithActions& ctx) {
        ctx.onTurnOn();
    }

    int on_count() const { return on_count_; }

    using transitions =
      std::tuple<Transition<Off,
                            Toggle,
                            On,
                            &SwitchHsmContextWithActions::onTurnOn,
                            &SwitchHsmContextWithActions::canTurnOn>,
                 Transition<On, Toggle, Off>>;

  private:
    int on_count_{};
};

using SwitchHsmWithActions = make_hsm_t<SwitchHsmContextWithActions>;

TEST_CASE("SwitchHsmWithActions") {
    using SwitchHsm = make_hsm_t<SwitchHsmContextWithActions>;
    SwitchHsm hsm;
    REQUIRE(std::holds_alternative<SwitchHsmContextWithActions::Off*>(
      hsm.current_state_));
    hsm.handle(SwitchHsmContextWithActions::Toggle());
    REQUIRE(hsm.on_count() == 1);
    REQUIRE(std::holds_alternative<SwitchHsmContextWithActions::On*>(
      hsm.current_state_));
    hsm.handle(SwitchHsmContextWithActions::Toggle());
}

// Traffic Light HSM

namespace TrafficLight {

struct LightContext {
    struct G1 {
        bool handle(LightContext&, ClockTickEvent& t) {
            if (t.ticks_ >= 30) {
                // exit action
                t.ticks_ = 0;
                return true;
            }
            return false;
        }
    };

    struct Y1 {
        bool handle(LightContext&, ClockTickEvent& t) {
            if (t.ticks_ >= 5) {
                // exit action
                t.ticks_ = 0;
                return true;
            }
            return false;
        }
        void entry(LightContext& t) { t.walk_pressed_ = false; }
    };

    struct G2 {
        bool handle(LightContext& l, ClockTickEvent& t) {
            if (t.ticks_ >= 60 || (l.walk_pressed_ && t.ticks_ >= 30)) {
                // exit action
                t.ticks_ = 0;
                return true;
            }
            return false;
        }
    };

    struct Y2 {
        bool handle(LightContext&, ClockTickEvent& t) {
            if (t.ticks_ >= 5) {
                // exit action
                t.ticks_ = 0;
                return true;
            }
            return false;
        }
    };

    bool walk_pressed_{};

    using transitions = std::tuple<ClockedTransition<G1, Y1>,
                                   ClockedTransition<Y1, G2>,
                                   ClockedTransition<G2, Y2>,
                                   ClockedTransition<Y2, G1>>;
};

// Emergency override trait. G1 and G2 will transition every five ticks. If
// walk_pressed_ is true, the light will transition to Y2 after 30 ticks. This
// will be part of a hierarchical state machine
struct EmergencyOverrideContext {
    struct BaseHandle {
        bool handle(EmergencyOverrideContext&, ClockTickEvent& t) {
            if (t.ticks_ >= 5) {
                // exit action
                t.ticks_ = 0;
                return true;
            }
            return false;
        }
    };
    struct G1 : BaseHandle {};

    struct Y1 : BaseHandle {};
    struct G2 : BaseHandle {};
    struct Y2 : BaseHandle {};

    bool walk_pressed_{};
    using transitions = std::tuple<ClockedTransition<G1, Y1>,
                                   ClockedTransition<Y1, G2>,
                                   ClockedTransition<G2, Y2>,
                                   ClockedTransition<Y2, G1>>;
};

// Parent HSM - TrafficLight
struct TrafficLightHsmContext {
    // Events
    struct EmergencySwitchOn {};
    struct EmergencySwitchOff {};

    using transitions = std::tuple<
      Transition<LightContext, EmergencySwitchOn, EmergencyOverrideContext>,
      Transition<EmergencyOverrideContext, EmergencySwitchOff, LightContext>>;
};

}

// Clocked HSM
TEST_CASE("TrafficLight") {
    using LightHsm = ClockedHsm<make_hsm_t<TrafficLight::LightContext>>;
    // check if transition from G1 to Y1 exists for ClockTickEvent
    // static_assert(
    //  has_valid_transition_v<TrafficLight::LightContext::G1, ClockTickEvent,
    //  LightHsm::transitions>);
    // LightHsm::transitions transitions;
    // transitions.blah();
    LightHsm hsm;
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::G1*>(
      hsm.current_state_));
    for (int i = 0; i < 30; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::Y1*>(
      hsm.current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::G2*>(
      hsm.current_state_));
    for (int i = 0; i < 60; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::Y2*>(
      hsm.current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::G1*>(
      hsm.current_state_));
}

TEST_CASE("TrafficLight with walk pressed") {
    using LightHsm = ClockedHsm<make_hsm_t<TrafficLight::LightContext>>;
    LightHsm hsm;
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::G1*>(
      hsm.current_state_));
    for (int i = 0; i < 30; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::Y1*>(
      hsm.current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::G2*>(
      hsm.current_state_));
    hsm.walk_pressed_ = true;
    for (int i = 0; i < 30; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::Y2*>(
      hsm.current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::G1*>(
      hsm.current_state_));
}

// // Test TrafficLightHsm
TEST_CASE("TrafficLightHsm") {
    using LightHsm = make_hsm_t<TrafficLight::LightContext>;
    using EmergencyOverrideHsm =
      make_hsm_t<TrafficLight::EmergencyOverrideContext>;
    using TrafficLightHsm =
      ClockedHsm<make_hsm_t<TrafficLight::TrafficLightHsmContext>>;

    TrafficLightHsm hsm;
    REQUIRE(std::holds_alternative<LightHsm*>(hsm.current_state_));
    hsm.handle(TrafficLight::TrafficLightHsmContext::EmergencySwitchOn());
    REQUIRE(std::holds_alternative<EmergencyOverrideHsm*>(hsm.current_state_));
    auto current_hsm = std::get<EmergencyOverrideHsm*>(hsm.current_state_);
    // Test that the hsm can handle ticks in the EmergencyOverrideHsm
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideContext::G1*>(
      current_hsm->current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideContext::Y1*>(
      current_hsm->current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideContext::G2*>(
      current_hsm->current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideContext::Y2*>(
      current_hsm->current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideContext::G1*>(
      current_hsm->current_state_));

    hsm.handle(TrafficLight::TrafficLightHsmContext::EmergencySwitchOff());
    REQUIRE(std::holds_alternative<LightHsm*>(hsm.current_state_));
}

// Test ConcurrentHsm
// Multiple TrafficLightHsm instances in parallel - sort of like a city street
namespace CityStreet {
struct Broadway {
    // Traffic on Park Ave
    using ParkAveLights = TrafficLight::LightContext;
    // Traffic on 5th Ave - specialize if needed
    using FifthAveLights = TrafficLight::LightContext;
    // create a policy by wrappint ThreadedExecutionPolicy around ClockedHsm
    template<typename T>
    using ThreadedClockedHsm = ThreadedExecutionPolicy<ClockedHsm<T>>;
    // Each have an independent clock that can be ticked
    using type =
      make_concurrent_hsm_t<ThreadedClockedHsm, ParkAveLights, FifthAveLights>;
};
}

// Test CityStreet
TEST_CASE("CityStreet") {
    using BroadwayHsm = CityStreet::Broadway::type;
    BroadwayHsm hsm;
    auto& park_ave = std::get<0>(hsm.hsms_);
    auto& fifth_ave = std::get<1>(hsm.hsms_);
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::G1*>(
      park_ave.current_state_));
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::G1*>(
      fifth_ave.current_state_));
    for (int i = 0; i < 30; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::Y1*>(
      park_ave.current_state_));
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::Y1*>(
      fifth_ave.current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::G2*>(
      park_ave.current_state_));
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::G2*>(
      fifth_ave.current_state_));
    for (int i = 0; i < 60; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::Y2*>(
      park_ave.current_state_));
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::Y2*>(
      fifth_ave.current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::G1*>(
      park_ave.current_state_));
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::G1*>(
      fifth_ave.current_state_));
}

// Test StateMachine SingleThreadedExecutionPolicy
TEST_CASE("Test SingleThreadedExecutionPolicy") {
    // apply policy
    using TrafficLightHsm =
      SingleThreadedExecutionPolicy<TrafficLight::TrafficLightHsmContext>;
    using LightHsm = make_hsm_t<TrafficLight::LightContext>;
    using EmergencyOverrideHsm =
      make_hsm_t<TrafficLight::EmergencyOverrideContext>;

    TrafficLightHsm hsm;

    hsm.send_event(TrafficLight::TrafficLightHsmContext::EmergencySwitchOn());
    REQUIRE(std::holds_alternative<LightHsm*>(hsm.current_state_));
    hsm.step();
    auto current_hsm = std::get<EmergencyOverrideHsm*>(hsm.current_state_);
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideContext::G1*>(
      current_hsm->current_state_));
    ClockTickEvent tick{};
    for (int i = 0; i < 5; i++) {
        hsm.send_event(tick);
    }
    for (int i = 0; i < 5; i++) {
        hsm.step();
    }
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideContext::Y1*>(
      current_hsm->current_state_));
}

// Test StateMachine ThreadedExecutionPolicy
TEST_CASE("Test ThreadedExecutionPolicy") {
    // apply policy
    using TrafficLightHsm =
      ThreadedExecutionPolicy<TrafficLight::TrafficLightHsmContext>;
    using LightHsm = make_hsm_t<TrafficLight::LightContext>;
    using EmergencyOverrideHsm =
      make_hsm_t<TrafficLight::EmergencyOverrideContext>;

    TrafficLightHsm hsm;
    hsm.start();
    hsm.send_event(TrafficLight::TrafficLightHsmContext::EmergencySwitchOn());
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    REQUIRE(std::holds_alternative<EmergencyOverrideHsm*>(hsm.current_state_));
    auto current_hsm = std::get<EmergencyOverrideHsm*>(hsm.current_state_);
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideContext::G1*>(
      current_hsm->current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.send_event(ClockTickEvent{});
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideContext::Y1*>(
      current_hsm->current_state_));

    // Switch off the emergency override
    hsm.send_event(TrafficLight::TrafficLightHsmContext::EmergencySwitchOff());
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    REQUIRE(std::holds_alternative<LightHsm*>(hsm.current_state_));
    hsm.stop();
}

// Test RealtimeExecutionPolicy
#ifdef __linux__
TEST_CASE("Test RealtimeExecutionPolicy") {
    using TrafficLightHsm =
      RealtimeExecutionPolicy<TrafficLight::TrafficLightHsmContext>;
    using EmergencyOverrideHsm =
      make_hsm_t<TrafficLight::EmergencyOverrideContext>;
    using LightHsm = make_hsm_t<TrafficLight::LightContext>;
    // write a better test than this
    TrafficLightHsm hsm;
    hsm.start();
    REQUIRE(std::holds_alternative<LightHsm*>(hsm.current_state_));
    hsm.send_event(TrafficLight::TrafficLightHsmContext::EmergencySwitchOn());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    REQUIRE(std::holds_alternative<EmergencyOverrideHsm*>(hsm.current_state_));
    auto current_hsm = std::get<EmergencyOverrideHsm*>(hsm.current_state_);
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideContext::G1*>(
      current_hsm->current_state_));
    ClockTickEvent tick{};
    for (int i = 0; i < 5; i++) {
        hsm.send_event(tick);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideContext::Y1*>(
      current_hsm->current_state_));
    hsm.stop();
}

// Test RealtimePeriodicExecutionPolicy - use traffic light HSM
TEST_CASE("Test PeriodicExecutionPolicy") {
    // default is to send a timer tick event every 1ms
    using TrafficLightHsm =
      RealtimePeriodicHsm<TrafficLight::TrafficLightHsmContext>;

    using LightHsm = make_hsm_t<TrafficLight::LightContext>;

    TrafficLightHsm hsm;
    hsm.start();
    REQUIRE(std::holds_alternative<LightHsm*>(hsm.current_state_));
    auto current_hsm = std::get<LightHsm*>(hsm.current_state_);
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::G1*>(
      current_hsm->current_state_));
    while (!(std::holds_alternative<TrafficLight::LightContext::Y1*>(
      current_hsm->current_state_)))
        ;
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::Y1*>(
      current_hsm->current_state_));
    while (!(std::holds_alternative<TrafficLight::LightContext::G2*>(
      current_hsm->current_state_)))
        ;
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::G2*>(
      current_hsm->current_state_));
    while (!(std::holds_alternative<TrafficLight::LightContext::Y2*>(
      current_hsm->current_state_)))
        ;
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::Y2*>(
      current_hsm->current_state_));
    while (!(std::holds_alternative<TrafficLight::LightContext::G1*>(
      current_hsm->current_state_)))
        ;
    REQUIRE(std::holds_alternative<TrafficLight::LightContext::G1*>(
      current_hsm->current_state_));
    hsm.stop();
}
#endif // __linux__

#include <iostream>
// Rewrite TrafficLightHsm with actions, guards, entry and exit actions
namespace TrafficLightAG {
struct LightContext {
    struct G1 {
        void entry(LightContext&, ClockTickEvent& t) { t.ticks_ = 0; }
        bool guard(LightContext&, ClockTickEvent& t) { return t.ticks_ >= 30; }
    };

    struct Y1 {
        bool guard(LightContext&, ClockTickEvent& t) { return t.ticks_ >= 5; }
    };

    struct G2 {
        void entry(LightContext&, ClockTickEvent& t) { t.ticks_ = 0; }
        bool guard(LightContext& l, ClockTickEvent& t) {
            return t.ticks_ >= 60 || (l.walk_pressed_ && t.ticks_ >= 30);
        }
    };

    struct Y2 {
        void entry(LightContext&, ClockTickEvent& t) { t.ticks_ = 0; }
        bool guard(LightContext&, ClockTickEvent& t) { return t.ticks_ >= 5; }
        void action(LightContext& l, ClockTickEvent&) {
            l.walk_pressed_ = false;
        }
    };
    using transitions = std::tuple<ClockedTransition<G1, Y1>,
                                   ClockedTransition<Y1, G2>,
                                   ClockedTransition<G2, Y2>,
                                   ClockedTransition<Y2, G1>>;
    bool walk_pressed_{};
};
}
// Test TrafficLightAG
TEST_CASE("TrafficLightAG") {
    using LightHsm = ClockedHsm<TrafficLightAG::LightContext>;
    LightHsm hsm;
    REQUIRE(std::holds_alternative<TrafficLightAG::LightContext::G1*>(
      hsm.current_state_));
    for (int i = 0; i < 30; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLightAG::LightContext::Y1*>(
      hsm.current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLightAG::LightContext::G2*>(
      hsm.current_state_));
    for (int i = 0; i < 60; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLightAG::LightContext::Y2*>(
      hsm.current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLightAG::LightContext::G1*>(
      hsm.current_state_));
}
