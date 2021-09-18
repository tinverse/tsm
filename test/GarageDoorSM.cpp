#include "GarageDoorSM.h"
#include "Observer.h"

#include <catch2/catch.hpp>

using tsm::AsyncExecWithObserver;
using tsm::BlockingObserver;
using tsm::SingleThreadedHsm;

using tsmtest::GarageDoorHsm;

/// A "Blocking" Observer with Async Execution Policy
template<typename StateType>
using AsyncBlockingObserver =
  tsm::AsyncExecWithObserver<StateType, BlockingObserver>;

///
/// GarageDoorHsm is the state machine definition. It has knowledge of the Hsm
/// hierarchy, its states, events and sub-Hsms if any. The relationships between
/// Hsms (parentHsm_) is also setup here. Mix the Async observer and the
/// GarageDoor Hsm to get an asynchronous garage Door state machine
/// that notifies a listener at the end of processing each event.
///
using GarageDoorHsmSeparateThread = AsyncBlockingObserver<GarageDoorHsm>;

TEST_CASE("TestGarageDoorSM - testGarageDoorSeparateThreadPolicy")
{
    auto sm = std::make_shared<GarageDoorHsmSeparateThread>();

    sm->startSM();

    sm->wait();
    REQUIRE(sm->getCurrentState() == &sm->DoorClosed);

    sm->sendEvent(sm->click_event);
    sm->wait();
    REQUIRE(sm->getCurrentState() == &sm->DoorOpening);

    sm->sendEvent(sm->sensor_hi_event);
    sm->wait();
    REQUIRE(sm->getCurrentState() == &sm->DoorOpen);

    sm->stopSM();
}

TEST_CASE("TestGarageDoorSM - testGarageDoorSingleThreadPolicy")
{
    auto sm = std::make_shared<SingleThreadedHsm<GarageDoorHsm>>();

    sm->sendEvent(sm->click_event);
    sm->sendEvent(sm->sensor_hi_event);

    sm->startSM();
    REQUIRE(sm->getCurrentState() == &sm->DoorClosed);

    sm->step();
    REQUIRE(sm->getCurrentState() == &sm->DoorOpening);

    sm->step();
    REQUIRE(sm->getCurrentState() == &sm->DoorOpen);

    sm->stopSM();
}
