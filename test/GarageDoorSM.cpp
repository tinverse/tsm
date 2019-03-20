#include "GarageDoorSM.h"
#include "Observer.h"

#include <catch2/catch.hpp>

using tsm::AsyncExecWithObserver;
using tsm::BlockingObserver;
using tsm::FsmExecutor;
using tsm::SingleThreadedFsm;

using tsmtest::GarageDoorDef;

/// A "Blocking" Observer with Async Execution Policy
template<typename StateType>
using AsyncBlockingObserver =
  tsm::AsyncExecWithObserver<StateType, BlockingObserver>;

///
/// GarageDoorDef is the state machine definition. It has knowledge of the Hsm
/// hierarchy, its states, events and sub-Hsms if any. The relationships between
/// Hsms (parentHsm_) is also setup here. Mix the Async observer and the
/// GarageDoor HsmDefinition to get an asynchronous garage door state machine
/// that notifies a listener at the end of processing each event.
///
using GarageDoorFsmSeparateThread =
  AsyncBlockingObserver<FsmExecutor<GarageDoorDef>>;

TEST_CASE("TestGarageDoorSM - testGarageDoorSeparateThreadPolicy")
{
    auto sm = std::make_shared<GarageDoorFsmSeparateThread>();
    auto smDef = std::static_pointer_cast<GarageDoorDef>(sm);

    sm->startSM();

    sm->wait();
    REQUIRE(sm->getCurrentState() == &smDef->doorClosed);

    sm->sendEvent(smDef->click_event);
    sm->wait();
    REQUIRE(sm->getCurrentState() == &smDef->doorOpening);

    sm->sendEvent(smDef->topSensor_event);
    sm->wait();
    REQUIRE(sm->getCurrentState() == &smDef->doorOpen);

    sm->stopSM();
}

TEST_CASE("TestGarageDoorSM - testGarageDoorSingleThreadPolicy")
{
    auto sm = std::make_shared<SingleThreadedFsm<GarageDoorDef>>();
    auto smDef = std::static_pointer_cast<GarageDoorDef>(sm);

    sm->sendEvent(smDef->click_event);
    sm->sendEvent(smDef->topSensor_event);

    sm->startSM();
    REQUIRE(sm->getCurrentState() == &sm->doorClosed);

    sm->step();
    REQUIRE(sm->getCurrentState() == &smDef->doorOpening);

    sm->step();
    REQUIRE(sm->getCurrentState() == &smDef->doorOpen);

    sm->stopSM();
}
