#include "TypedHsm.h"

// Test the HSM using the Catch2 framework
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

using namespace tsm;
// Example usage
// A Switch HSM
struct SwitchHsmTraits {
    // Events
    struct Toggle {};

    // States
    struct Off {};
    struct On {};

    using transitions =
      std::tuple<Transition<Off, Toggle, On>, Transition<On, Toggle, Off>>;
};

using SwitchHsm = make_hsm_t<SwitchHsmTraits>;

TEST_CASE("SwitchHsm") {
    SwitchHsm hsm;
    REQUIRE(std::holds_alternative<SwitchHsmTraits::Off*>(hsm.current_state_));
    hsm.handle(SwitchHsmTraits::Toggle());
    REQUIRE(std::holds_alternative<SwitchHsmTraits::On*>(hsm.current_state_));
}

// Traffic Light HSM

namespace TrafficLight {

struct LightTraits : clocked_trait {
    struct G1 {
        void exit(LightTraits& t) { t.ticks_ = 0; }

        auto guard(LightTraits& t) -> bool {
            if (t.ticks_ >= 30) {
                return true;
            }
            return false;
        };
    };

    struct Y1 {
        void entry(LightTraits& t) { t.walk_pressed_ = false; }

        void exit(LightTraits& t) { t.ticks_ = 0; }

        auto guard(LightTraits& t) -> bool { return t.ticks_ >= 5; };
    };

    struct G2 {
        void exit(LightTraits& t) { t.ticks_ = 0; }
        auto guard(LightTraits& t) -> bool {
            return (!t.walk_pressed_) ? t.ticks_ >= 60 : t.ticks_ >= 30;
        };
    };

    struct Y2 {
        void exit(LightTraits& t) { t.ticks_ = 0; }

        auto guard(LightTraits& t) -> bool { return t.ticks_ >= 5; };

        auto action(LightTraits& t) -> void { t.walk_pressed_ = false; }
    };

    bool walk_pressed_{};

    using transitions =
      std::tuple<ClockedTransition<G1, Y1, decltype(&G1::guard)>,
                 ClockedTransition<Y1, G2, decltype(&Y1::guard)>,
                 ClockedTransition<G2, Y2, decltype(&G2::guard)>,
                 ClockedTransition<Y2, G1, decltype(&Y2::guard)>>;
};

// Emergency override trait. G1 and G2 will transition every five ticks. If
// walk_pressed_ is true, the light will transition to Y2 after 30 ticks. This
// will be part of a hierarchical state machine
struct EmergencyOverrideTraits : clocked_trait {
    struct G1 {
        void exit(EmergencyOverrideTraits& t) { t.ticks_ = 0; }

        auto guard(EmergencyOverrideTraits& t) -> bool {
            return t.ticks_ >= 5;
        };
    };
    struct Y1 {
        void exit(EmergencyOverrideTraits& t) { t.ticks_ = 0; }
        auto guard(EmergencyOverrideTraits& t) -> bool {
            return t.ticks_ >= 5;
        };
    };
    struct G2 {
        void exit(EmergencyOverrideTraits& t) { t.ticks_ = 0; }
        auto guard(EmergencyOverrideTraits& t) -> bool {
            return t.ticks_ >= 5;
        };
    };
    struct Y2 {
        void exit(EmergencyOverrideTraits& t) { t.ticks_ = 0; }
        auto guard(EmergencyOverrideTraits& t) -> bool {
            return t.ticks_ >= 5;
        };
    };

    bool walk_pressed_{};
    using transitions =
      std::tuple<ClockedTransition<G1, Y1, decltype(&G1::guard)>,
                 ClockedTransition<Y1, G2, decltype(&Y1::guard)>,
                 ClockedTransition<G2, Y2, decltype(&G2::guard)>,
                 ClockedTransition<Y2, G1, decltype(&Y2::guard)>>;
};

// Parent HSM - TrafficLight
struct TrafficLightHsmTraits {
    // Events
    struct EmergencySwitchOn {};
    struct EmergencySwitchOff {};

    using transitions = std::tuple<
      Transition<LightTraits, EmergencySwitchOn, EmergencyOverrideTraits>,
      Transition<EmergencyOverrideTraits, EmergencySwitchOff, LightTraits>>;
};

}

// Clocked HSM
TEST_CASE("TrafficLight") {
    using LightHsm = make_hsm_t<TrafficLight::LightTraits>;
    LightHsm hsm;
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::G1*>(
      hsm.current_state_));
    for (int i = 0; i < 30; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::Y1*>(
      hsm.current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::G2*>(
      hsm.current_state_));
    for (int i = 0; i < 60; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::Y2*>(
      hsm.current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::G1*>(
      hsm.current_state_));
}

TEST_CASE("TrafficLight with walk pressed") {
    using LightHsm = make_hsm_t<TrafficLight::LightTraits>;
    LightHsm hsm;
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::G1*>(
      hsm.current_state_));
    for (int i = 0; i < 30; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::Y1*>(
      hsm.current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::G2*>(
      hsm.current_state_));
    hsm.walk_pressed_ = true;
    for (int i = 0; i < 30; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::Y2*>(
      hsm.current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.tick();
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::G1*>(
      hsm.current_state_));
}

// // Test TrafficLightHsm
TEST_CASE("TrafficLightHsm") {
    using LightHsm = make_hsm_t<TrafficLight::LightTraits>;
    using EmergencyOverrideHsm =
      make_hsm_t<TrafficLight::EmergencyOverrideTraits>;
    using TrafficLightHsm = make_hsm_t<TrafficLight::TrafficLightHsmTraits>;

    TrafficLightHsm hsm;
    REQUIRE(std::holds_alternative<LightHsm*>(hsm.current_state_));
    hsm.handle(TrafficLight::TrafficLightHsmTraits::EmergencySwitchOn());
    REQUIRE(std::holds_alternative<EmergencyOverrideHsm*>(hsm.current_state_));
    auto current_hsm = std::get<EmergencyOverrideHsm*>(hsm.current_state_);
    // Test that the hsm can handle ticks in the EmergencyOverrideHsm
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideTraits::G1*>(
      current_hsm->current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.handle(ClockTickEvent{});
    }
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideTraits::Y1*>(
      current_hsm->current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.handle(ClockTickEvent{});
    }
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideTraits::G2*>(
      current_hsm->current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.handle(ClockTickEvent{});
    }
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideTraits::Y2*>(
      current_hsm->current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.handle(ClockTickEvent{});
    }
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideTraits::G1*>(
      current_hsm->current_state_));

    hsm.handle(TrafficLight::TrafficLightHsmTraits::EmergencySwitchOff());
    REQUIRE(std::holds_alternative<LightHsm*>(hsm.current_state_));
}

// Test OrthogonalHsm
// Multiple TrafficLightHsm instances in parallel - sort of like a city street
namespace CityStreet {
struct Broadway {
    // Traffic on Park Ave
    using ParkAveLights = TrafficLight::LightTraits;
    // Traffic on 5th Ave - specialize if needed
    using FifthAveLights = TrafficLight::LightTraits;
    using type = make_orthogonal_hsm_t<ParkAveLights, FifthAveLights>;
};
}

// Test CityStreet
TEST_CASE("CityStreet") {
    using BroadwayHsm = CityStreet::Broadway::type;
    BroadwayHsm hsm;
    auto& park_ave = std::get<0>(hsm.hsms_);
    auto& fifth_ave = std::get<1>(hsm.hsms_);
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::G1*>(
      park_ave.current_state_));
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::G1*>(
      fifth_ave.current_state_));
    for (int i = 0; i < 30; i++) {
        hsm.handle(ClockTickEvent{});
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::Y1*>(
      park_ave.current_state_));
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::Y1*>(
      fifth_ave.current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.handle(ClockTickEvent{});
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::G2*>(
      park_ave.current_state_));
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::G2*>(
      fifth_ave.current_state_));
    for (int i = 0; i < 60; i++) {
        hsm.handle(ClockTickEvent{});
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::Y2*>(
      park_ave.current_state_));
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::Y2*>(
      fifth_ave.current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.handle(ClockTickEvent{});
    }
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::G1*>(
      park_ave.current_state_));
    REQUIRE(std::holds_alternative<TrafficLight::LightTraits::G1*>(
      fifth_ave.current_state_));
}

// Test StateMachine SingleThreadedExecutionPolicy
TEST_CASE("Test SingleThreadedExecutionPolicy") {
    // apply policy
    using TrafficLightHsm = SingleThreadedExecutionPolicy<
      make_hsm_t<TrafficLight::TrafficLightHsmTraits>>;
    using LightHsm = make_hsm_t<TrafficLight::LightTraits>;
    using EmergencyOverrideHsm =
      make_hsm_t<TrafficLight::EmergencyOverrideTraits>;

    TrafficLightHsm hsm;

    hsm.send_event(TrafficLight::TrafficLightHsmTraits::EmergencySwitchOn());
    REQUIRE(std::holds_alternative<LightHsm*>(hsm.current_state_));
    // assert true 'step()'
    REQUIRE(hsm.step());
    auto current_hsm = std::get<EmergencyOverrideHsm*>(hsm.current_state_);
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideTraits::G1*>(
      current_hsm->current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.send_event(ClockTickEvent{});
    }
    for (int i = 0; i < 5; i++) {
        hsm.step();
    }
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideTraits::Y1*>(
      current_hsm->current_state_));
}

// Test StateMachine ThreadedExecutionPolicy
TEST_CASE("Test ThreadedExecutionPolicy") {
    // apply policy
    using TrafficLightHsm =
      ThreadedBlockingObserver<make_hsm_t<TrafficLight::TrafficLightHsmTraits>>;
    using LightHsm = make_hsm_t<TrafficLight::LightTraits>;
    using EmergencyOverrideHsm =
      make_hsm_t<TrafficLight::EmergencyOverrideTraits>;

    TrafficLightHsm hsm;
    hsm.start();
    hsm.send_event(TrafficLight::TrafficLightHsmTraits::EmergencySwitchOn());
    hsm.wait();
    REQUIRE(std::holds_alternative<EmergencyOverrideHsm*>(hsm.current_state_));
    auto current_hsm = std::get<EmergencyOverrideHsm*>(hsm.current_state_);
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideTraits::G1*>(
      current_hsm->current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.send_event(ClockTickEvent{});
    }
    hsm.wait();
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideTraits::Y1*>(
      current_hsm->current_state_));

    // Switch off the emergency override
    hsm.send_event(TrafficLight::TrafficLightHsmTraits::EmergencySwitchOff());
    hsm.wait();
    REQUIRE(std::holds_alternative<LightHsm*>(hsm.current_state_));
    hsm.stop();
}

// Test RealTimeExecutionPolicy
#ifdef __linux__
TEST_CASE("Test RealTimeExecutionPolicy") {
    using TrafficLightHsm =
      RealTimeExecutionPolicy<make_hsm_t<TrafficLight::TrafficLightHsmTraits>>;
    using EmergencyOverrideHsm =
      make_hsm_t<TrafficLight::EmergencyOverrideTraits>;

    TrafficLightHsm hsm;
    hsm.send_event(TrafficLight::TrafficLightHsmTraits::EmergencySwitchOn());
    hsm.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    REQUIRE(std::holds_alternative<EmergencyOverrideHsm*>(hsm.current_state_));
    auto current_hsm = std::get<EmergencyOverrideHsm*>(hsm.current_state_);
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideTraits::G1*>(
      current_hsm->current_state_));
    for (int i = 0; i < 5; i++) {
        hsm.send_event(ClockTickEvent{});
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    REQUIRE(std::holds_alternative<TrafficLight::EmergencyOverrideTraits::Y1*>(
      current_hsm->current_state_));
    hsm.stop();
}

// Test RealTimePeriodicExecutionPolicy - use traffic light HSM
TEST_CASE("Test PeriodicExecutionPolicy") {
    using TrafficLightHsm =
      PeriodicExecutionPolicy<make_hsm_t<TrafficLight::TrafficLightHsmTraits>>;

    using LightHsm = make_hsm_t<TrafficLight::LightTraits>;

    TrafficLightHsm hsm;
    hsm.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    REQUIRE(std::holds_alternative<LightHsm*>(hsm.current_state_));
    hsm.stop();
}
#endif // __linux__
