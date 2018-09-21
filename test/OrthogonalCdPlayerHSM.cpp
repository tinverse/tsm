#include <glog/logging.h>
#include <gtest/gtest.h>

#include "CdPlayerHSM.h"
#include "StateMachineTest.h"

using tsm::OrthogonalHSM;

using tsmtest::CdPlayerController;
using tsmtest::CdPlayerHSM;
using tsmtest::ErrorHSM;

using OrthogonalCdPlayerHSM =
  OrthogonalHSM<CdPlayerHSM<CdPlayerController>, ErrorHSM>;

struct TestOrthogonalCdPlayerHSM : public testing::Test
{
    TestOrthogonalCdPlayerHSM()
      : testing::Test()
      , sm(std::make_shared<OrthogonalCdPlayerHSM>(
          "CD Player Orthogonal HSM",
          eventQueue,
          std::make_shared<CdPlayerHSM<CdPlayerController>>("CD Player HSM",
                                                            eventQueue),
          std::make_shared<ErrorHSM>("Error HSM", eventQueue)))
    {}

    // The StateMachine
    // Starting State: Empty
    shared_ptr<OrthogonalCdPlayerHSM> sm;

    // Event Queue
    EventQueue<Event> eventQueue;
};

TEST_F(TestOrthogonalCdPlayerHSM, testTransitions)
{

    auto cdPlayerHSM =
      std::static_pointer_cast<CdPlayerHSM<CdPlayerController>>(sm->getHsm1());

    auto& Playing = cdPlayerHSM->Playing;

    auto errorHSM = std::static_pointer_cast<ErrorHSM>(sm->getHsm2());

    ASSERT_EQ(Playing->getParent(), cdPlayerHSM.get());
    ASSERT_EQ(sm.get(), cdPlayerHSM->getParent());
    ASSERT_EQ(sm.get(), errorHSM->getParent());

    sm->onEntry();

    ASSERT_EQ(errorHSM->getCurrentState(), errorHSM->AllOk);

    eventQueue.addEvent(cdPlayerHSM->cd_detected);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), cdPlayerHSM->Stopped);

    eventQueue.addEvent(cdPlayerHSM->play);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), Playing);

    ASSERT_EQ(Playing->getCurrentState(), Playing->Song1);

    eventQueue.addEvent(Playing->next_song);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song2);

    eventQueue.addEvent(cdPlayerHSM->pause);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), cdPlayerHSM->Paused);
    ASSERT_EQ(Playing->getCurrentState(), nullptr);

    eventQueue.addEvent(cdPlayerHSM->end_pause);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), cdPlayerHSM->Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song1);

    eventQueue.addEvent(cdPlayerHSM->stop);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), cdPlayerHSM->Stopped);
    ASSERT_EQ(Playing->getCurrentState(), nullptr);

    // same as TestCdPlayerHSM::testTransitions upto this point

    eventQueue.addEvent(errorHSM->error);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), cdPlayerHSM->Stopped);
    ASSERT_EQ(errorHSM->getCurrentState(), errorHSM->ErrorMode);

    eventQueue.addEvent(errorHSM->recover);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(cdPlayerHSM->getCurrentState(), cdPlayerHSM->Stopped);
    ASSERT_EQ(errorHSM->getCurrentState(), errorHSM->AllOk);

    sm->onExit();
}
