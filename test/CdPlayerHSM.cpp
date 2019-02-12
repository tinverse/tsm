#include "CdPlayerHSM.h"
#include "Observer.h"

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
struct TestCdPlayerHSM : public testing::Test
{
    TestCdPlayerHSM()
      : testing::Test()
    {}
    virtual ~TestCdPlayerHSM() {}
};

TEST_F(TestCdPlayerHSM, testTransitionsSeparateThreadPolicy)
{

    CdPlayerHSMSeparateThread sm;

    auto& Playing = sm.Playing;

    ASSERT_EQ(Playing.getParent(), &sm);

    sm.startSM();

    sm.wait();
    sm.sendEvent(sm.cd_detected);

    sm.wait();
    ASSERT_EQ(sm.getCurrentState(), &sm.Stopped);

    sm.sendEvent(sm.play);
    sm.wait();
    ASSERT_EQ(sm.getCurrentState(), &Playing);
    ASSERT_EQ(Playing.getCurrentState(), &Playing.Song1);

    sm.sendEvent(Playing.next_song);
    sm.wait();
    ASSERT_EQ(sm.getCurrentState(), &Playing);
    ASSERT_EQ(Playing.getCurrentState(), &Playing.Song2);

    sm.sendEvent(sm.pause);
    sm.wait();
    ASSERT_EQ(sm.getCurrentState(), &sm.Paused);
    ASSERT_EQ(Playing.getCurrentState(), &Playing.Song2);

    sm.sendEvent(sm.end_pause);
    sm.wait();
    ASSERT_EQ(sm.getCurrentState(), &sm.Playing);
    ASSERT_EQ(Playing.getCurrentState(), &Playing.Song2);

    sm.sendEvent(Playing.next_song);
    sm.wait();
    ASSERT_EQ(sm.getCurrentState(), &sm.Playing);
    ASSERT_EQ(Playing.getCurrentState(), &Playing.Song3);

    sm.sendEvent(sm.stop_event);
    sm.wait();
    ASSERT_EQ(sm.getCurrentState(), &sm.Stopped);
    ASSERT_EQ(Playing.getCurrentState(), nullptr);

    sm.stopSM();
}

TEST_F(TestCdPlayerHSM, testTransitionsParentThreadPolicy)
{
    CdPlayerHSMParentThread sm;

    auto& Playing = sm.Playing;

    ASSERT_EQ(Playing.getParent(), (State*)&sm);

    sm.startSM();

    sm.sendEvent(sm.cd_detected);
    sm.step();
    ASSERT_EQ(sm.getCurrentState(), &sm.Stopped);

    sm.sendEvent(sm.play);
    sm.step();
    ASSERT_EQ(sm.getCurrentState(), &Playing);

    ASSERT_EQ(Playing.getCurrentState(), &Playing.Song1);

    sm.sendEvent(Playing.next_song);
    sm.step();
    ASSERT_EQ(sm.getCurrentState(), &Playing);
    ASSERT_EQ(Playing.getCurrentState(), &Playing.Song2);

    sm.sendEvent(sm.pause);
    sm.step();
    ASSERT_EQ(sm.getCurrentState(), &sm.Paused);
    ASSERT_EQ(Playing.getCurrentState(), &Playing.Song2);

    sm.sendEvent(sm.end_pause);
    sm.step();
    ASSERT_EQ(sm.getCurrentState(), &sm.Playing);
    ASSERT_EQ(Playing.getCurrentState(), &Playing.Song2);

    sm.sendEvent(Playing.next_song);
    sm.step();
    ASSERT_EQ(sm.getCurrentState(), &sm.Playing);
    ASSERT_EQ(Playing.getCurrentState(), &Playing.Song3);

    sm.sendEvent(sm.stop_event);
    sm.step();
    ASSERT_EQ(sm.getCurrentState(), &sm.Stopped);
    ASSERT_EQ(Playing.getCurrentState(), nullptr);

    sm.stopSM();
}

TEST_F(TestCdPlayerHSM, testCallingStepOnParentThreadPolicyEmptyEventQueue)
{
    CdPlayerHSMParentThread sm;

    sm.startSM();
    sm.sendEvent(sm.cd_detected);
    sm.step();
    ASSERT_EQ(sm.getCurrentState(), &sm.Stopped);

    // Event queue is empty now. Nothing should change
    sm.step();
    ASSERT_EQ(sm.getCurrentState(), &sm.Stopped);

    sm.stopSM();
}

TEST_F(TestCdPlayerHSM, testEventQueueInterruptedException)
{
    CdPlayerHSMParentThread sm;

    sm.startSM();
    sm.sendEvent(sm.cd_detected);
    sm.step();
    ASSERT_EQ(sm.getCurrentState(), &sm.Stopped);

    sm.stopSM();
}
