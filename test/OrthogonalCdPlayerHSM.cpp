#include "CdPlayerHSM.h"
#include "Observer.h"

#include <gtest/gtest.h>

using tsm::AsyncExecutionPolicy;
using tsm::BlockingObserver;
using tsm::OrthogonalStateMachine;
using tsm::ParentThreadExecutionPolicy;

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

/// A "Blocking" Observer with Async Execution Policy
template<typename StateType>
using AsyncBlockingObserver =
  tsm::AsyncExecWithObserver<StateType, BlockingObserver>;

using OrthogonalCdPlayerHSMSeparateThread =
  AsyncBlockingObserver<OrthogonalCdPlayerHSM>;

using OrthogonalCdPlayerHSMParentThread =
  ParentThreadExecutionPolicy<OrthogonalCdPlayerHSM>;

struct TestOrthogonalCdPlayerHSM : public testing::Test
{
    TestOrthogonalCdPlayerHSM()
      : testing::Test()
    {}
    ~TestOrthogonalCdPlayerHSM() {}
};

TEST_F(TestOrthogonalCdPlayerHSM, testOrthogonalHSMSeparateThread)
{
    auto sm = std::make_shared<OrthogonalCdPlayerHSMSeparateThread>();

    auto* cdPlayerHSM = &sm->getHsm1();

    auto* Playing = &cdPlayerHSM->Playing;

    auto* errorHSM = &sm->getHsm2();

    ASSERT_EQ(Playing->getParent(), cdPlayerHSM);
    ASSERT_EQ(sm.get(), cdPlayerHSM->getParent());
    ASSERT_EQ(sm.get(), errorHSM->getParent());

    sm->startSM();

    sm->wait();
    ASSERT_EQ(errorHSM->getCurrentState(), &errorHSM->AllOk);

    sm->sendEvent(cdPlayerHSM->cd_detected);
    sm->wait();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), &cdPlayerHSM->Stopped);

    sm->sendEvent(cdPlayerHSM->play);
    sm->wait();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), Playing);

    ASSERT_EQ(Playing->getCurrentState(), &Playing->Song1);

    sm->sendEvent(Playing->next_song);
    sm->wait();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), Playing);
    ASSERT_EQ(Playing->getCurrentState(), &Playing->Song2);

    sm->sendEvent(cdPlayerHSM->pause);
    sm->wait();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), &cdPlayerHSM->Paused);
    ASSERT_EQ(Playing->getCurrentState(), &Playing->Song2);

    sm->sendEvent(cdPlayerHSM->end_pause);
    sm->wait();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), &cdPlayerHSM->Playing);
    ASSERT_EQ(Playing->getCurrentState(), &Playing->Song2);

    sm->sendEvent(cdPlayerHSM->stop_event);
    sm->wait();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), &cdPlayerHSM->Stopped);
    ASSERT_EQ(Playing->getCurrentState(), nullptr);

    // same as TestCdPlayerDef::testTransitions upto this point
    sm->sendEvent(errorHSM->error);
    sm->wait();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), &cdPlayerHSM->Stopped);
    ASSERT_EQ(errorHSM->getCurrentState(), &errorHSM->ErrorMode);

    sm->sendEvent(errorHSM->recover);
    sm->wait();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), &cdPlayerHSM->Stopped);
    ASSERT_EQ(errorHSM->getCurrentState(), &errorHSM->AllOk);

    sm->stopSM();
}

TEST_F(TestOrthogonalCdPlayerHSM, testOrthogonalHSMParentThread)
{

    auto sm = std::make_shared<OrthogonalCdPlayerHSMParentThread>();

    auto* cdPlayerHSM = &sm->getHsm1();

    auto* Playing = &cdPlayerHSM->Playing;

    auto* errorHSM = &sm->getHsm2();

    ASSERT_EQ(Playing->getParent(), cdPlayerHSM);
    ASSERT_EQ(sm.get(), cdPlayerHSM->getParent());
    ASSERT_EQ(sm.get(), errorHSM->getParent());

    sm->startSM();

    ASSERT_EQ(errorHSM->getCurrentState(), &errorHSM->AllOk);

    sm->sendEvent(cdPlayerHSM->cd_detected);
    sm->step();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), &cdPlayerHSM->Stopped);

    sm->sendEvent(cdPlayerHSM->play);
    sm->step();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), Playing);

    ASSERT_EQ(Playing->getCurrentState(), &Playing->Song1);

    sm->sendEvent(Playing->next_song);
    sm->step();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), Playing);
    ASSERT_EQ(Playing->getCurrentState(), &Playing->Song2);

    sm->sendEvent(cdPlayerHSM->pause);
    sm->step();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), &cdPlayerHSM->Paused);
    ASSERT_EQ(Playing->getCurrentState(), &Playing->Song2);

    sm->sendEvent(cdPlayerHSM->end_pause);
    sm->step();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), &cdPlayerHSM->Playing);
    ASSERT_EQ(Playing->getCurrentState(), &Playing->Song2);

    sm->sendEvent(cdPlayerHSM->stop_event);
    sm->step();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), &cdPlayerHSM->Stopped);
    ASSERT_EQ(Playing->getCurrentState(), nullptr);

    // same as TestCdPlayerDef::testTransitions upto this point
    sm->sendEvent(errorHSM->error);
    sm->step();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), &cdPlayerHSM->Stopped);
    ASSERT_EQ(errorHSM->getCurrentState(), &errorHSM->ErrorMode);

    sm->sendEvent(errorHSM->recover);
    sm->step();
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), &cdPlayerHSM->Stopped);
    ASSERT_EQ(errorHSM->getCurrentState(), &errorHSM->AllOk);

    sm->stopSM();
}
