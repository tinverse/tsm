#include "Event.h"
#include "EventQueue.h"
#include "State.h"
#include "StateMachineDef.h"

#include <memory>

#include <glog/logging.h>
#include <gtest/gtest.h>

using tsm::Event;
using tsm::State;
using tsm::StateMachineDef;

using std::make_shared;
using std::shared_ptr;

// struct SimpleSM : public StateMachineDef<SimpleSM>
//{
//    using StateMachineDef<SimpleSM>::uniqueId_;
//
//    SimpleSM()
//      : StateMachineDef<SimpleSM>("Simple SM")
//      , e(uniqueId_.next())
//      , s(make_shared<State>("Test State", uniqueId_.next()))
//    {}
//
//    bool operator=(SimpleSM const& rhs)
//    {
//        return (e.id == rhs.e.id) && (s->id == rhs.s->id);
//    }
//
//    Event e;
//    shared_ptr<State> s;
//};

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

// TEST_F(TestState, testTwoInstancesAreEqual)
//{
//    auto sm1 = make_shared<SimpleSM>();
//    auto sm2 = make_shared<SimpleSM>();
//
//    ASSERT_EQ(sm1, sm2);
//}

int
main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
