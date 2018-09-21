#include "CdPlayerHSM.h"

using tsmtest::CdPlayerController;
using tsmtest::CdPlayerDef;

using CdPlayerHSMSeparateThread =
  StateMachineWithExecutionPolicy<StateMachine<CdPlayerDef<CdPlayerController>>,
                                  SeparateThreadExecutionPolicy>;

using CdPlayerHSMParentThread =
  StateMachineWithExecutionPolicy<StateMachine<CdPlayerDef<CdPlayerController>>,
                                  ParentThreadExecutionPolicy>;

// TODO(sriram): Test no end state
// Model interruptions to workflow
// For e.g. As door is opening or closing, one of the sensors
// detects an obstacle.
struct TestCdPlayerHSM : public testing::Test
{
    TestCdPlayerHSM()
      : testing::Test()
    {}
};

TEST_F(TestCdPlayerHSM, testTransitionsSeparateThreadPolicy)
{
    auto sm = std::make_shared<CdPlayerHSMSeparateThread>();

    auto& Playing = sm->Playing;

    ASSERT_EQ(Playing->getParent(), sm.get());

    sm->onEntry();

    sm->sendEvent(sm->cd_detected);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(sm->getCurrentState(), sm->Stopped);

    sm->sendEvent(sm->play);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(sm->getCurrentState(), Playing);

    ASSERT_EQ(Playing->getCurrentState(), Playing->Song1);

    sm->sendEvent(Playing->next_song);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(sm->getCurrentState(), Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song2);

    sm->sendEvent(sm->pause);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(sm->getCurrentState(), sm->Paused);
    ASSERT_EQ(Playing->getCurrentState(), nullptr);

    sm->sendEvent(sm->end_pause);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(sm->getCurrentState(), sm->Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song1);

    sm->sendEvent(sm->stop_event);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(sm->getCurrentState(), sm->Stopped);
    ASSERT_EQ(Playing->getCurrentState(), nullptr);

    sm->onExit();
}

TEST_F(TestCdPlayerHSM, testTransitionsParentThreadPolicy)
{
    auto sm = std::make_shared<CdPlayerHSMParentThread>();

    auto& Playing = sm->Playing;

    ASSERT_EQ(Playing->getParent(), sm.get());

    sm->onEntry();

    sm->sendEvent(sm->cd_detected);
    sm->step();
    ASSERT_EQ(sm->getCurrentState(), sm->Stopped);

    sm->sendEvent(sm->play);
    sm->step();
    ASSERT_EQ(sm->getCurrentState(), Playing);

    ASSERT_EQ(Playing->getCurrentState(), Playing->Song1);

    sm->sendEvent(Playing->next_song);
    sm->step();
    ASSERT_EQ(sm->getCurrentState(), Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song2);

    sm->sendEvent(sm->pause);
    sm->step();
    ASSERT_EQ(sm->getCurrentState(), sm->Paused);
    ASSERT_EQ(Playing->getCurrentState(), nullptr);

    sm->sendEvent(sm->end_pause);
    sm->step();
    ASSERT_EQ(sm->getCurrentState(), sm->Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song1);

    sm->sendEvent(sm->stop_event);
    sm->step();
    ASSERT_EQ(sm->getCurrentState(), sm->Stopped);
    ASSERT_EQ(Playing->getCurrentState(), nullptr);

    sm->onExit();
}
