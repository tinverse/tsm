#include "TrafficLightHsm.h"
#include "Observer.h"
#include "TimedExecutionPolicy.h"

#include <catch2/catch.hpp>

#include <chrono>

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
/// Mix the Async observer and the / TrafficLight Hsm to get an asynchronous
/// traffic light state machine / that notifies a listener at the end of
/// processing each event.
///
using AsyncTrafficLightTimedHsm =
  TimedExecutionPolicy<AsyncBlockingObserver<TrafficLightHsm>,
                       tsm::ThreadSleepTimer,
                       std::chrono::microseconds>;

TEST_CASE("AsyncTrafficLightTimedHsm - testAsyncTrafficLightStatesNoWalk")
{
    using namespace std::chrono_literals;
    auto sm = std::make_shared<AsyncTrafficLightTimedHsm>(500us);
    std::vector<AsyncTrafficLightTimedHsm::LightState*> states{
        &sm->G1, &sm->Y1, &sm->G2, &sm->Y2
    };
    sm->startSM();
    // are we cycling through all states?
    for (auto* state : states) {
        REQUIRE(sm->getCurrentState() == state);
        for (uint64_t i = 0; i <= state->getLimit() + 1; ++i) {
            sm->wait();
        }
    }
    sm->stopSM();
}

TEST_CASE("AsyncTrafficLightTimedHsm - testAsyncTrafficLightStatesWalkPressed")
{
    using namespace std::chrono_literals;
    auto sm = std::make_shared<AsyncTrafficLightTimedHsm>(500us);
    std::vector<AsyncTrafficLightTimedHsm::LightState*> states{
        &sm->G1, &sm->Y1, &sm->G2, &sm->Y2
    };
    sm->startSM();
    // are we cycling through all states?
    for (auto* state : states) {
        REQUIRE(sm->getCurrentState()->id == state->id);
        for (uint64_t i = 0; i <= state->getLimit() + 1; ++i) {
            // Press the walk signal 28 ticks into G1
            if ((state->name == "G1") && (sm->ticks_ == 28)) {
                sm->pressWalk();
            }
            // break on the 30th tick in G2 if walk signal is pressed
            if (sm->walkPressed && (state->name == "G2") &&
                (sm->ticks_ >= TrafficLightHsm::G2WALK)) {
                sm->wait();
                break;
            }
            sm->wait();
        }
    }
    sm->stopSM();
}

using SynchronousTrafficLightHsm =
  tsm::SingleThreadedExecutionPolicy<TrafficLightHsm>;

std::ostream&
operator<<(std::ostream& os, NamedState* s)
{
    os << s->name;
    return os;
}

TEST_CASE("SynchronousTrafficLightHsm- testSyncTrafficLightHsmNoWalk")
{
    using namespace std::chrono_literals;
    auto sm = std::make_shared<SynchronousTrafficLightHsm>();
    std::vector<SynchronousTrafficLightHsm::LightState*> states{
        &sm->G1, &sm->Y1, &sm->G2, &sm->Y2
    };
    sm->startSM();
    // are we cycling through all states?
    for (auto* state : states) {
        REQUIRE(sm->getCurrentState() == state);
        for (uint64_t i = 0; i <= state->getLimit(); ++i) {
            sm->sendEvent(sm->timer_event);
            sm->step();
        }
    }
    sm->stopSM();
}

using TrafficLightTimedHsm = tsm::ClockedMooreHsm<TrafficLightHsm,
                                                  tsm::ThreadSleepTimer,
                                                  std::chrono::microseconds>;

TEST_CASE("TrafficLightTimedHsm - testTrafficLightStatesNoWalk")
{
    using namespace std::chrono_literals;
    auto sm = std::make_shared<TrafficLightTimedHsm>(1us);
    std::vector<TrafficLightTimedHsm::LightState*> states{
        &sm->G1, &sm->Y1, &sm->G2, &sm->Y2
    };
    sm->startSM();
    // are we cycling through all states?
    for (auto* state : states) {
        REQUIRE(sm->getCurrentState() == state);
        for (uint64_t i = 0; i <= state->getLimit(); ++i) {
            sm->step();
        }
    }
    sm->stopSM();
}

TEST_CASE("TrafficLightTimedHsm - testTrafficLightStatesWithWalkPressed")
{
    using namespace std::chrono_literals;
    auto sm = std::make_shared<TrafficLightTimedHsm>(1us);
    std::vector<TrafficLightTimedHsm::LightState*> states{
        &sm->G1, &sm->Y1, &sm->G2, &sm->Y2
    };
    sm->startSM();
    // are we cycling through all states?
    for (auto* state : states) {
        REQUIRE(sm->getCurrentState() == state);
        for (uint64_t i = 0; i <= state->getLimit(); ++i) {
            // Press the walk signal 15 ticks into G1
            if ((state->name == "G1") && (i == 15)) {
                sm->pressWalk();
            }
            // break on the 30th tick in G2 if walk signal is pressed
            if (sm->walkPressed && (state->name == "G2") &&
                (i >= TrafficLightHsm::G2WALK)) {
                sm->step();
                break;
            }
            sm->step();
        }
    }
    sm->stopSM();
}
