#include "CdPlayerHSM.h"
#include "Observer.h"

using tsmtest::CdPlayerController;
using tsmtest::CdPlayerDef;

using tsm::AsyncExecWithObserver;
using tsm::AsyncStateMachine;
using tsm::BlockingObserver;
using tsm::StateMachine;
using tsm::StateMachineWithExecutionPolicy;

/// A "Blocking" Observer with Async Execution Policy
template<typename StateType>
using AsyncBlockingObserver =
  tsm::AsyncExecWithObserver<StateType, BlockingObserver>;

using CdPlayerHSMSeparateThread =
  StateMachineWithExecutionPolicy<HSMBehavior<CdPlayerDef<CdPlayerController>>,
                                  AsyncBlockingObserver>;

using CdPlayerHSMParentThread = StateMachine<CdPlayerDef<CdPlayerController>>;

// TODO(sriram): Test no end state
// Model interruptions to workflow
// For e.g. As door is opening or closing, one of the sensors
// detects an obstacle.
struct TestCdPlayerHSM : public testing::Test
{
    TestCdPlayerHSM()
      : testing::Test()
    {}
    ~TestCdPlayerHSM()
    { // TODO(sriram): ugh! Fix Id generation
        tsm::UniqueId::reset();
    }
};

TEST_F(TestCdPlayerHSM, testTransitionsSeparateThreadPolicy)
{

    shared_ptr<CdPlayerHSMSeparateThread> sm(
      std::make_shared<CdPlayerHSMSeparateThread>());

    auto& Playing = sm->Playing;

    ASSERT_EQ(Playing->getParent(), sm.get());

    sm->startSM();

    sm->wait();
    sm->sendEvent(sm->cd_detected);

    sm->wait();
    ASSERT_EQ(sm->getCurrentState(), sm->Stopped);

    sm->sendEvent(sm->play);
    sm->wait();
    ASSERT_EQ(sm->getCurrentState(), Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song1);

    sm->sendEvent(Playing->next_song);
    sm->wait();
    ASSERT_EQ(sm->getCurrentState(), Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song2);

    sm->sendEvent(sm->pause);
    sm->wait();
    ASSERT_EQ(sm->getCurrentState(), sm->Paused);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song2);

    sm->sendEvent(sm->end_pause);
    sm->wait();
    ASSERT_EQ(sm->getCurrentState(), sm->Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song2);

    sm->sendEvent(Playing->next_song);
    sm->wait();
    ASSERT_EQ(sm->getCurrentState(), sm->Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song3);

    sm->sendEvent(sm->stop_event);
    sm->wait();
    ASSERT_EQ(sm->getCurrentState(), sm->Stopped);
    ASSERT_EQ(Playing->getCurrentState(), nullptr);

    sm->stopSM();
}

TEST_F(TestCdPlayerHSM, testTransitionsParentThreadPolicy)
{

    shared_ptr<CdPlayerHSMParentThread> sm(
      std::make_shared<CdPlayerHSMParentThread>());

    auto& Playing = sm->Playing;

    ASSERT_EQ(Playing->getParent(), sm.get());

    sm->startSM();

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

    LOG(INFO) << "******Current State:" << Playing->getCurrentState()->name;
    sm->sendEvent(sm->pause);
    sm->step();
    ASSERT_EQ(sm->getCurrentState(), sm->Paused);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song2);

    sm->sendEvent(sm->end_pause);
    sm->step();
    ASSERT_EQ(sm->getCurrentState(), sm->Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song2);

    sm->sendEvent(Playing->next_song);
    sm->step();
    ASSERT_EQ(sm->getCurrentState(), sm->Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song3);

    sm->sendEvent(sm->stop_event);
    sm->step();
    ASSERT_EQ(sm->getCurrentState(), sm->Stopped);
    ASSERT_EQ(Playing->getCurrentState(), nullptr);
    sm->stopSM();
}
