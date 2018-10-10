#include "GarageDoorSM.h"
#include "Observer.h"

using tsm::AsyncExecWithObserver;
using tsm::BlockingObserver;
using tsm::SimpleStateMachine;
using tsm::StateMachine;

using tsmtest::GarageDoorDef;

/// A "Blocking" Observer with Async Execution Policy
template<typename StateType>
using AsyncBlockingObserver =
  tsm::AsyncExecWithObserver<StateType, BlockingObserver>;

struct TestGarageDoorSM : public ::testing::Test
{
    TestGarageDoorSM()
      : testing::Test()
    {}
    ~TestGarageDoorSM() { tsm::UniqueId::reset(); }
};

///
/// Mix the Async observer and the GarageDoor StateMachine to get an
/// asynchronous garage door state machine that notifies a listener at the end
/// of processing each event.
///
using GarageDoorHSMSeparateThread =
  AsyncBlockingObserver<StateMachine<GarageDoorDef>>;

TEST_F(TestGarageDoorSM, testGarageDoorSeparateThreadPolicy)
{
    auto sm = std::make_shared<GarageDoorHSMSeparateThread>();
    auto smDef = std::static_pointer_cast<GarageDoorDef>(sm);

    sm->startSM();

    sm->wait();
    ASSERT_EQ(sm->getCurrentState(), &smDef->doorClosed);

    sm->sendEvent(smDef->click_event);
    sm->wait();
    ASSERT_EQ(sm->getCurrentState(), &smDef->doorOpening);

    sm->sendEvent(smDef->topSensor_event);
    sm->wait();
    ASSERT_EQ(sm->getCurrentState(), &smDef->doorOpen);

    sm->stopSM();
}

TEST_F(TestGarageDoorSM, testGarageDoorParentThreadPolicy)
{
    auto sm = std::make_shared<SimpleStateMachine<GarageDoorDef>>();
    auto smDef = std::static_pointer_cast<GarageDoorDef>(sm);

    sm->sendEvent(smDef->click_event);
    sm->sendEvent(smDef->topSensor_event);

    sm->startSM();
    ASSERT_EQ(sm->getCurrentState(), &sm->doorClosed);

    sm->step();
    ASSERT_EQ(sm->getCurrentState(), &smDef->doorOpening);

    sm->step();
    ASSERT_EQ(sm->getCurrentState(), &smDef->doorOpen);

    sm->stopSM();
}
