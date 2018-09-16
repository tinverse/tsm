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
    ASSERT_EQ(sm->getCurrentState(), sm->doorClosed)
      << " CurrentState: " << sm->getCurrentState()->name;
    eventQueue.addEvent(sm->click_event);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(sm->doorOpening, sm->getCurrentState())
      << " CurrentState: " << sm->getCurrentState()->name;
    eventQueue.addEvent(sm->topSensor_event);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(sm->doorOpen, sm->getCurrentState())
      << " CurrentState: " << sm->getCurrentState()->name;
    sm->OnExit();
}
