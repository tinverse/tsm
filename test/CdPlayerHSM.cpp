#include "CdPlayerHSM.h"
#include "Observer.h"

#include <catch2/catch.hpp>

using tsmtest::CdPlayerController;
using tsmtest::CdPlayerDef;

using tsm::AsyncExecWithObserver;
using tsm::AsyncStateMachine;
using tsm::BlockingObserver;
using tsm::SimpleStateMachine;
using tsm::StateMachine;

/// A "Blocking" Observer with Async Execution Policy
template<typename StateType>
using AsyncBlockingObserver =
  tsm::AsyncExecWithObserver<StateType, BlockingObserver>;

using CdPlayerHSMSeparateThread =
  AsyncBlockingObserver<StateMachine<CdPlayerDef<CdPlayerController>>>;

using CdPlayerHSMParentThread =
  SimpleStateMachine<CdPlayerDef<CdPlayerController>>;

// TODO(sriram): Test no end state
// Model interruptions to workflow
// For e.g. As door is opening or closing, one of the sensors
// detects an obstacle.
TEST_CASE("TestCdPlayerHSM - testTransitionsSeparateThreadPolicy")
{

    CdPlayerHSMSeparateThread sm;

    auto& Playing = sm.Playing;

    REQUIRE(Playing.getParent() == &sm);

    sm.startSM();

    sm.wait();
    sm.sendEvent(sm.cd_detected);

    sm.wait();
    REQUIRE(sm.getCurrentState() == &sm.Stopped);

    sm.sendEvent(sm.play);
    sm.wait();
    REQUIRE(sm.getCurrentState() == &Playing);
    REQUIRE(Playing.getCurrentState() == &Playing.Song1);

    sm.sendEvent(Playing.next_song);
    sm.wait();
    REQUIRE(sm.getCurrentState() == &Playing);
    REQUIRE(Playing.getCurrentState() == &Playing.Song2);

    sm.sendEvent(sm.pause);
    sm.wait();
    REQUIRE(sm.getCurrentState() == &sm.Paused);
    REQUIRE(Playing.getCurrentState() == &Playing.Song2);

    sm.sendEvent(sm.end_pause);
    sm.wait();
    REQUIRE(sm.getCurrentState() == &sm.Playing);
    REQUIRE(Playing.getCurrentState() == &Playing.Song2);

    sm.sendEvent(Playing.next_song);
    sm.wait();
    REQUIRE(sm.getCurrentState() == &sm.Playing);
    REQUIRE(Playing.getCurrentState() == &Playing.Song3);

    sm.sendEvent(sm.stop_event);
    sm.wait();
    REQUIRE(sm.getCurrentState() == &sm.Stopped);
    REQUIRE(Playing.getCurrentState() == nullptr);

    sm.stopSM();
}

TEST_CASE("TestCdPlayerHSM - testTransitionsParentThreadPolicy")
{
    CdPlayerHSMParentThread sm;

    auto& Playing = sm.Playing;

    REQUIRE(Playing.getParent() == (State*)&sm);

    sm.startSM();

    sm.sendEvent(sm.cd_detected);
    sm.step();
    REQUIRE(sm.getCurrentState() == &sm.Stopped);

    sm.sendEvent(sm.play);
    sm.step();
    REQUIRE(sm.getCurrentState() == &Playing);

    REQUIRE(Playing.getCurrentState() == &Playing.Song1);

    sm.sendEvent(Playing.next_song);
    sm.step();
    REQUIRE(sm.getCurrentState() == &Playing);
    REQUIRE(Playing.getCurrentState() == &Playing.Song2);

    sm.sendEvent(sm.pause);
    sm.step();
    REQUIRE(sm.getCurrentState() == &sm.Paused);
    REQUIRE(Playing.getCurrentState() == &Playing.Song2);

    sm.sendEvent(sm.end_pause);
    sm.step();
    REQUIRE(sm.getCurrentState() == &sm.Playing);
    REQUIRE(Playing.getCurrentState() == &Playing.Song2);

    sm.sendEvent(Playing.next_song);
    sm.step();
    REQUIRE(sm.getCurrentState() == &sm.Playing);
    REQUIRE(Playing.getCurrentState() == &Playing.Song3);

    sm.sendEvent(sm.stop_event);
    sm.step();
    REQUIRE(sm.getCurrentState() == &sm.Stopped);
    REQUIRE(Playing.getCurrentState() == nullptr);

    sm.stopSM();
}

TEST_CASE(
  "TestCdPlayerHSM - testCallingStepOnParentThreadPolicyEmptyEventQueue")
{
    CdPlayerHSMParentThread sm;

    sm.startSM();
    sm.sendEvent(sm.cd_detected);
    sm.step();
    REQUIRE(sm.getCurrentState() == &sm.Stopped);

    // Event queue is empty now. Nothing should change
    sm.step();
    REQUIRE(sm.getCurrentState() == &sm.Stopped);

    sm.stopSM();
}

TEST_CASE("TestCdPlayerHSM - testEventQueueInterruptedException")
{
    CdPlayerHSMParentThread sm;

    sm.startSM();
    sm.sendEvent(sm.cd_detected);
    sm.step();
    REQUIRE(sm.getCurrentState() == &sm.Stopped);

    sm.stopSM();
}
