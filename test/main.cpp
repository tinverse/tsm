#include "TestMachines.h"
#include "tsm.h"

#include <memory>

#include <glog/logging.h>
#include <gtest/gtest.h>

using tsm::Event;
using tsm::State;
using tsm::StateMachineDef;

using tsmtest::AHsmDef;
using tsmtest::BHsmDef;

class TestState : public testing::Test
{
  public:
    TestState()
      : state_("Dummy")
    {}
    ~TestState() override = default;
    void SetUp() override {}
    void TearDown() override {}

  protected:
    State state_;
};

TEST_F(TestState, Construct)
{
    DLOG(INFO) << "Test" << std::endl;
    EXPECT_EQ(state_.name, "Dummy");
}

struct TestStateMachineProperties : public testing::Test
{
    virtual ~TestStateMachineProperties() {}
};

TEST_F(TestStateMachineProperties, testMachineExitsWhenReachingStopState)
{
    SimpleStateMachine<AHsmDef> sm;
    sm.startSM();
    ASSERT_EQ(&sm.s1, sm.getCurrentState());

    sm.sendEvent(sm.e1);
    sm.step();
    ASSERT_EQ(&sm.s2, sm.getCurrentState());

    sm.sendEvent(sm.e2_in);
    sm.step();
    ASSERT_EQ(&sm.bHsmDef, sm.getCurrentState());

    sm.sendEvent(sm.e2_out);
    sm.step();
    ASSERT_EQ(&sm.s3, sm.getCurrentState())
      << "exp: " << sm.s3.name << " act: " << sm.getCurrentState()->name;

    // Send unknown event
    Event randomEvent;
    sm.sendEvent(randomEvent);
    sm.step();

    sm.sendEvent(sm.end_event);
    sm.step();
    ASSERT_EQ(nullptr, sm.getCurrentState());
}

int
main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
