#include <glog/logging.h>
#include <gtest/gtest.h>

#include "CdPlayerHSM.h"

using tsm::EventQueueT;
using tsm::OrthogonalStateMachine;

using tsmtest::CdPlayerController;
using tsmtest::CdPlayerDef;
using tsmtest::ErrorHSM;

struct OrthogonalCdPlayerHSM
  : public OrthogonalStateMachine<CdPlayerDef<CdPlayerController>, ErrorHSM>
{
    OrthogonalCdPlayerHSM()
      : OrthogonalStateMachine<CdPlayerDef<CdPlayerController>, ErrorHSM>(
          "CD Player Orthogonal HSM")
    {}
    virtual ~OrthogonalCdPlayerHSM() = default;
};

using OrthogonalCdPlayerHSMSeparateThread =
  StateMachineWithExecutionPolicy<OrthogonalCdPlayerHSM,
                                  SeparateThreadExecutionPolicy>;

using OrthogonalCdPlayerHSMParentThread =
  StateMachineWithExecutionPolicy<OrthogonalCdPlayerHSM,
                                  ParentThreadExecutionPolicy>;

struct TestOrthogonalCdPlayerHSM : public testing::Test
{
    TestOrthogonalCdPlayerHSM()
      : testing::Test()
    {}
};

TEST_F(TestOrthogonalCdPlayerHSM, testOrthogonalHSMSeparateThread)
{
    shared_ptr<OrthogonalCdPlayerHSMSeparateThread> sm(
      std::make_shared<OrthogonalCdPlayerHSMSeparateThread>());

    auto cdPlayerHSM =
      std::static_pointer_cast<CdPlayerDef<CdPlayerController>>(sm->getHsm1());

    auto& Playing = cdPlayerHSM->Playing;

    auto errorHSM = std::static_pointer_cast<ErrorHSM>(sm->getHsm2());

    ASSERT_EQ(Playing->getParent(), cdPlayerHSM.get());
    ASSERT_EQ(sm.get(), cdPlayerHSM->getParent());
    ASSERT_EQ(sm.get(), errorHSM->getParent());

    sm->onEntry();

    ASSERT_EQ(errorHSM->getCurrentState(), errorHSM->AllOk);

    sm->sendEvent(cdPlayerHSM->cd_detected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), cdPlayerHSM->Stopped);

    sm->sendEvent(cdPlayerHSM->play);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), Playing);

    ASSERT_EQ(Playing->getCurrentState(), Playing->Song1);

    sm->sendEvent(Playing->next_song);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song2);

    sm->sendEvent(cdPlayerHSM->pause);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), cdPlayerHSM->Paused);
    ASSERT_EQ(Playing->getCurrentState(), nullptr);

    sm->sendEvent(cdPlayerHSM->end_pause);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), cdPlayerHSM->Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song1);

    sm->sendEvent(cdPlayerHSM->stop_event);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), cdPlayerHSM->Stopped);
    ASSERT_EQ(Playing->getCurrentState(), nullptr);

    // same as TestCdPlayerDef::testTransitions upto this point
    sm->sendEvent(errorHSM->error);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), cdPlayerHSM->Stopped);
    ASSERT_EQ(errorHSM->getCurrentState(), errorHSM->ErrorMode);

    sm->sendEvent(errorHSM->recover);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), cdPlayerHSM->Stopped);
    ASSERT_EQ(errorHSM->getCurrentState(), errorHSM->AllOk);

    sm->onExit();
}

TEST_F(TestOrthogonalCdPlayerHSM, testOrthogonalHSMParentThread)
{
    shared_ptr<OrthogonalCdPlayerHSMParentThread> sm(
      std::make_shared<OrthogonalCdPlayerHSMParentThread>());

    auto cdPlayerHSM =
      std::static_pointer_cast<CdPlayerDef<CdPlayerController>>(sm->getHsm1());

    auto& Playing = cdPlayerHSM->Playing;

    auto errorHSM = std::static_pointer_cast<ErrorHSM>(sm->getHsm2());

    ASSERT_EQ(Playing->getParent(), cdPlayerHSM.get());
    ASSERT_EQ(sm.get(), cdPlayerHSM->getParent());
    ASSERT_EQ(sm.get(), errorHSM->getParent());

    sm->onEntry();

    ASSERT_EQ(errorHSM->getCurrentState(), errorHSM->AllOk);

    sm->sendEvent(cdPlayerHSM->cd_detected);
    sm->step();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), cdPlayerHSM->Stopped);

    sm->sendEvent(cdPlayerHSM->play);
    sm->step();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), Playing);

    ASSERT_EQ(Playing->getCurrentState(), Playing->Song1);

    sm->sendEvent(Playing->next_song);
    sm->step();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song2);

    sm->sendEvent(cdPlayerHSM->pause);
    sm->step();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), cdPlayerHSM->Paused);
    ASSERT_EQ(Playing->getCurrentState(), nullptr);

    sm->sendEvent(cdPlayerHSM->end_pause);
    sm->step();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), cdPlayerHSM->Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song1);

    sm->sendEvent(cdPlayerHSM->stop_event);
    sm->step();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), cdPlayerHSM->Stopped);
    ASSERT_EQ(Playing->getCurrentState(), nullptr);

    // same as TestCdPlayerDef::testTransitions upto this point
    sm->sendEvent(errorHSM->error);
    sm->step();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), cdPlayerHSM->Stopped);
    ASSERT_EQ(errorHSM->getCurrentState(), errorHSM->ErrorMode);

    sm->sendEvent(errorHSM->recover);
    sm->step();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), cdPlayerHSM->Stopped);
    ASSERT_EQ(errorHSM->getCurrentState(), errorHSM->AllOk);

    sm->onExit();
}
