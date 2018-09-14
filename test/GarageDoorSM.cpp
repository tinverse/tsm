#include "GarageDoorSM.h"

using tsmtest::GarageDoorSM;

struct TestGarageDoorSM : public ::testing::Test
{
    TestGarageDoorSM()
      : testing::Test()
      , sm(std::make_shared<GarageDoorSM>("Garage Door SM", eventQueue))
    {}
    // Event Queue
    EventQueue<Event> eventQueue;

    // The StateMachine
    // Starting State: doorClosed
    // Stop State: doorOpen
    std::shared_ptr<GarageDoorSM> sm;
};

TEST_F(TestGarageDoorSM, testGarageDoor)
{
    sm->OnEntry();
    EXPECT_EQ(sm->getCurrentState(), sm->doorClosed);
    eventQueue.addEvent(sm->click_event);
    EXPECT_EQ(sm->doorOpening, sm->getCurrentState());
    eventQueue.addEvent(sm->topSensor_event);
    EXPECT_EQ(sm->doorOpen, sm->getCurrentState());
    sm->OnExit();
}
