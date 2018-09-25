#include "GarageDoorSM.h"

using tsmtest::GarageDoorDef;

using GarageDoorHSMSeparateThread =
  StateMachineExecutionPolicy<StateMachine<GarageDoorDef>,
                                  AsyncExecutionPolicy>;

using GarageDoorHSMParentThread =
  StateMachineExecutionPolicy<StateMachine<GarageDoorDef>,
                                  ParentThreadExecutionPolicy>;

// Start State declared first
const shared_ptr<State> GarageDoorDef::doorClosed =
  std::make_shared<State>("Door Closed");

struct TestGarageDoorSM : public ::testing::Test
{
    TestGarageDoorSM()
      : testing::Test()
    {}
};

TEST_F(TestGarageDoorSM, testGarageDoorSeparateThreadPolicy)
{
    auto sm = std::make_shared<GarageDoorHSMSeparateThread>();
    auto smDef = std::static_pointer_cast<GarageDoorDef>(sm);
    sm->onEntry();
    ASSERT_EQ(sm->getCurrentState(), smDef->doorClosed)
      << " CurrentState: " << sm->getCurrentState()->name;
    sm->sendEvent(smDef->click_event);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(smDef->doorOpening, sm->getCurrentState())
      << " CurrentState: " << sm->getCurrentState()->name;
    sm->sendEvent(smDef->topSensor_event);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(smDef->doorOpen, sm->getCurrentState())
      << " CurrentState: " << sm->getCurrentState()->name;
    sm->onExit();
}

TEST_F(TestGarageDoorSM, testGarageDoorParentThreadPolicy)
{
    auto sm = std::make_shared<GarageDoorHSMParentThread>();
    auto smDef = std::static_pointer_cast<GarageDoorDef>(sm);

    sm->sendEvent(smDef->click_event);
    sm->sendEvent(smDef->topSensor_event);

    sm->onEntry();
    ASSERT_EQ(sm->getCurrentState(), sm->doorClosed)
      << " CurrentState: " << sm->getCurrentState()->name;
    sm->step();
    ASSERT_EQ(smDef->doorOpening, sm->getCurrentState())
      << " CurrentState: " << sm->getCurrentState()->name;
    sm->step();
    ASSERT_EQ(smDef->doorOpen, sm->getCurrentState())
      << " CurrentState: " << sm->getCurrentState()->name;
    sm->onExit();
}
