#include "TrafficLightHsm.h"
#include "Observer.h"
#include "TimedExecutionPolicy.h"

#include <catch2/catch.hpp>

using tsm::BlockingObserver;
using tsm::Hsm;
using tsm::SingleThreadedHsm;
using tsm::TimedExecutionPolicy;

using tsmtest::TrafficLightHsm;

/// A "Blocking" Observer with Async Execution Policy
template<typename StateType>
using AsyncBlockingObserver =
  tsm::AsyncExecWithObserver<StateType, BlockingObserver>;

///
/// GarageDoorHsm is the state machine definition. It has knowledge of the Hsm
/// hierarchy, its states, events and sub-Hsms if any. The relationships between
/// Hsms (parentHsm_) is also setup here. Mix the Async observer and the
/// TrafficLight Hsm to get an asynchronous traffic light state machine
/// that notifies a listener at the end of processing each event.
///
using AsyncTrafficLightTimedHsm =
  TimedExecutionPolicy<AsyncBlockingObserver<TrafficLightHsm>,
                       tsm::ThreadSleepTimer>;

TEST_CASE("AsyncTrafficLightTimedHsm - testTrafficLightStatesNoWalk")
{
    using namespace std::chrono_literals;
    auto sm = std::make_shared<AsyncTrafficLightTimedHsm>(1ms);
    std::vector<AsyncTrafficLightTimedHsm::LightState*> states{
        &sm->G1, &sm->Y1, &sm->G2, &sm->Y2
    };
    sm->startSM();
    // are we cycling through all states?
    for (auto* state : states) {
        REQUIRE(sm->getCurrentState()->id == state->id);
        for (uint64_t i = 1; i <= state->getLimit(); i++) {
            sm->wait();
        }
        sm->wait();
    }
    sm->stopSM();
}

TEST_CASE("AsyncTrafficLightTimedHsm - testTrafficLightStatesWithWalkPressed")
{
    using namespace std::chrono_literals;
    auto sm = std::make_shared<AsyncTrafficLightTimedHsm>(1ms);
    std::vector<AsyncTrafficLightTimedHsm::LightState*> states{
        &sm->G1, &sm->Y1, &sm->G2, &sm->Y2
    };
    sm->startSM();
    // are we cycling through all states?
    for (auto* state : states) {
        REQUIRE(sm->getCurrentState()->id == state->id);
        for (uint64_t i = 1; i <= state->getLimit(); i++) {
            // Press the walk signal 28 ticks into G1
            if ((state->name == "G1") && (sm->ticks_ == 28)) {
                sm->pressWalk();
            }
            // break on the 30th tick in G2 if walk signal is pressed
            if (sm->walkPressed && (state->name == "G2") &&
                (sm->ticks_ >= 30)) {
                break;
            }
            sm->wait();
        }
        sm->wait();
    }
    sm->stopSM();
}
using TrafficLightTimedHsm =
  TimedExecutionPolicy<SingleThreadedHsm<TrafficLightHsm>,
                       tsm::ThreadSleepTimer>;

TEST_CASE("TrafficLightTimedHsm - testTrafficLightStatesNoWalk")
{
    using namespace std::chrono_literals;
    auto sm = std::make_shared<TrafficLightTimedHsm>(1ms);
    std::vector<TrafficLightTimedHsm::LightState*> states{
        &sm->G1, &sm->Y1, &sm->G2, &sm->Y2
    };
    sm->startSM();
    // are we cycling through all states?
    for (auto* state : states) {
        REQUIRE(sm->getCurrentState() == state);
        for (uint64_t i = 1; i <= state->getLimit(); i++) {
            sm->step();
        }
        sm->step();
    }
    sm->stopSM();
}

TEST_CASE("TrafficLightTimedHsm - testTrafficLightStatesWithWalkPressed")
{
    using namespace std::chrono_literals;
    auto sm = std::make_shared<TrafficLightTimedHsm>(1ms);
    std::vector<TrafficLightTimedHsm::LightState*> states{
        &sm->G1, &sm->Y1, &sm->G2, &sm->Y2
    };
    sm->startSM();
    // are we cycling through all states?
    for (auto* state : states) {
        REQUIRE(sm->getCurrentState()->id == state->id);
        for (uint64_t i = 1; i <= state->getLimit(); i++) {
            // Press the walk signal 15 ticks into G1
            if ((state->name == "G1") && (sm->ticks_ == 15)) {
                sm->pressWalk();
            }
            // break on the 30th tick in G2 if walk signal is pressed
            if (sm->walkPressed && (state->name == "G2") &&
                (sm->ticks_ >= 30)) {
                break;
            }
            sm->step();
        }
        sm->step();
    }
    sm->stopSM();
}
