#include "CdPlayerHSM.h"

using tsmtest::CdPlayerController;
using tsmtest::CdPlayerHSM;

// TODO(sriram): Test no end state
// Model interruptions to workflow
// For e.g. As door is opening or closing, one of the sensors
// detects an obstacle.
struct TestCdPlayerHSM : public testing::Test
{
    TestCdPlayerHSM()
      : testing::Test()
      , sm(std::make_shared<CdPlayerHSM<CdPlayerController>>("CD Player HSM",
                                                             eventQueue))
    {}
    // The StateMachine
    // Starting State: Empty
    shared_ptr<CdPlayerHSM<CdPlayerController>> sm;

    // Event Queue
    EventQueue<Event> eventQueue;
};

TEST_F(TestCdPlayerHSM, testTransitions)
{
    auto& Playing = sm->Playing;

    ASSERT_EQ(Playing->getParent(), sm.get());

    sm->OnEntry();

    eventQueue.addEvent(sm->cd_detected);
    ASSERT_EQ(sm->getCurrentState(), sm->Stopped);

    eventQueue.addEvent(sm->play);
    ASSERT_EQ(sm->getCurrentState(), Playing);

    ASSERT_EQ(Playing->getCurrentState(), Playing->Song1);

    eventQueue.addEvent(Playing->next_song);
    ASSERT_EQ(sm->getCurrentState(), Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song2);

    eventQueue.addEvent(sm->pause);
    ASSERT_EQ(sm->getCurrentState(), sm->Paused);
    ASSERT_EQ(Playing->getCurrentState(), nullptr);

    eventQueue.addEvent(sm->end_pause);
    ASSERT_EQ(sm->getCurrentState(), sm->Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song1);

    eventQueue.addEvent(sm->stop);
    ASSERT_EQ(sm->getCurrentState(), sm->Stopped);
    ASSERT_EQ(Playing->getCurrentState(), nullptr);

    sm->OnExit();
}
