#include "CdPlayerHSM.h"
#include "Observer.h"

#include <catch2/catch.hpp>

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

TEST_CASE("TestOrthogonalCdPlayerHSM - testOrthogonalHSMSeparateThread")
{
    auto sm = std::make_shared<OrthogonalCdPlayerHSMSeparateThread>();

    auto* cdPlayerHSM = &sm->getHsm1();

    auto* Playing = &cdPlayerHSM->Playing;

    auto* errorHSM = &sm->getHsm2();

    REQUIRE(Playing->getParent() == cdPlayerHSM);
    REQUIRE(sm.get() == cdPlayerHSM->getParent());
    REQUIRE(sm.get() == errorHSM->getParent());

    sm->startSM();

    sm->wait();
    REQUIRE(errorHSM->getCurrentState() == &errorHSM->AllOk);

    sm->sendEvent(cdPlayerHSM->cd_detected);
    sm->wait();
    REQUIRE(cdPlayerHSM->getCurrentState() == &cdPlayerHSM->Stopped);

    sm->sendEvent(cdPlayerHSM->play);
    sm->wait();
    REQUIRE(cdPlayerHSM->getCurrentState() == Playing);

    REQUIRE(Playing->getCurrentState() == &Playing->Song1);

    sm->sendEvent(Playing->next_song);
    sm->wait();
    REQUIRE(cdPlayerHSM->getCurrentState() == Playing);
    REQUIRE(Playing->getCurrentState() == &Playing->Song2);

    sm->sendEvent(cdPlayerHSM->pause);
    sm->wait();
    REQUIRE(cdPlayerHSM->getCurrentState() == &cdPlayerHSM->Paused);
    REQUIRE(Playing->getCurrentState() == &Playing->Song2);

    sm->sendEvent(cdPlayerHSM->end_pause);
    sm->wait();
    REQUIRE(cdPlayerHSM->getCurrentState() == &cdPlayerHSM->Playing);
    REQUIRE(Playing->getCurrentState() == &Playing->Song2);

    sm->sendEvent(cdPlayerHSM->stop_event);
    sm->wait();
    REQUIRE(cdPlayerHSM->getCurrentState() == &cdPlayerHSM->Stopped);
    REQUIRE(Playing->getCurrentState() == nullptr);

    // same as TestCdPlayerDef::testTransitions upto this point
    sm->sendEvent(errorHSM->error);
    sm->wait();
    REQUIRE(cdPlayerHSM->getCurrentState() == &cdPlayerHSM->Stopped);
    REQUIRE(errorHSM->getCurrentState() == &errorHSM->ErrorMode);

    sm->sendEvent(errorHSM->recover);
    sm->wait();
    REQUIRE(cdPlayerHSM->getCurrentState() == &cdPlayerHSM->Stopped);
    REQUIRE(errorHSM->getCurrentState() == &errorHSM->AllOk);

    sm->stopSM();
}

TEST_CASE("TestOrthogonalCdPlayerHSM - testOrthogonalHSMParentThread")
{

    auto sm = std::make_shared<OrthogonalCdPlayerHSMParentThread>();

    auto* cdPlayerHSM = &sm->getHsm1();

    auto* Playing = &cdPlayerHSM->Playing;

    auto* errorHSM = &sm->getHsm2();

    REQUIRE(Playing->getParent() == cdPlayerHSM);
    REQUIRE(sm.get() == cdPlayerHSM->getParent());
    REQUIRE(sm.get() == errorHSM->getParent());

    sm->startSM();

    REQUIRE(errorHSM->getCurrentState() == &errorHSM->AllOk);

    sm->sendEvent(cdPlayerHSM->cd_detected);
    sm->step();
    REQUIRE(cdPlayerHSM->getCurrentState() == &cdPlayerHSM->Stopped);

    sm->sendEvent(cdPlayerHSM->play);
    sm->step();
    REQUIRE(cdPlayerHSM->getCurrentState() == Playing);

    REQUIRE(Playing->getCurrentState() == &Playing->Song1);

    sm->sendEvent(Playing->next_song);
    sm->step();
    REQUIRE(cdPlayerHSM->getCurrentState() == Playing);
    REQUIRE(Playing->getCurrentState() == &Playing->Song2);

    sm->sendEvent(cdPlayerHSM->pause);
    sm->step();
    REQUIRE(cdPlayerHSM->getCurrentState() == &cdPlayerHSM->Paused);
    REQUIRE(Playing->getCurrentState() == &Playing->Song2);

    sm->sendEvent(cdPlayerHSM->end_pause);
    sm->step();
    REQUIRE(cdPlayerHSM->getCurrentState() == &cdPlayerHSM->Playing);
    REQUIRE(Playing->getCurrentState() == &Playing->Song2);

    sm->sendEvent(cdPlayerHSM->stop_event);
    sm->step();
    REQUIRE(cdPlayerHSM->getCurrentState() == &cdPlayerHSM->Stopped);
    REQUIRE(Playing->getCurrentState() == nullptr);

    // same as TestCdPlayerDef::testTransitions upto this point
    sm->sendEvent(errorHSM->error);
    sm->step();
    REQUIRE(cdPlayerHSM->getCurrentState() == &cdPlayerHSM->Stopped);
    REQUIRE(errorHSM->getCurrentState() == &errorHSM->ErrorMode);

    sm->sendEvent(errorHSM->recover);
    sm->step();
    REQUIRE(cdPlayerHSM->getCurrentState() == &cdPlayerHSM->Stopped);
    REQUIRE(errorHSM->getCurrentState() == &errorHSM->AllOk);

    sm->stopSM();
}
