#include <future>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <set>
#include <utility>

#include "CdPlayerHSM.h"
#include "Event.h"
#include "EventQueue.h"
#include "State.h"
#include "StateMachineTest.h"
#include "tsm.h"

using tsm::Event;
using tsm::EventQueue;
using tsm::OrthogonalHSM;
using tsm::State;
using tsm::StateMachine;

using tsmtest::CdPlayerController;
using tsmtest::CdPlayerHSM;
using tsmtest::StateMachineTest;

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

class TestEventQueue : public testing::Test
{
  public:
    ~TestEventQueue() override = default;

  protected:
    EventQueue<Event> eq_;
    Event e1;
};

TEST_F(TestEventQueue, testSingleEvent)
{
    auto f1 = std::async(&EventQueue<Event>::nextEvent, &eq_);

    std::thread t1(&EventQueue<Event>::addEvent, &eq_, e1);

    // Use the same threads to retrieve events
    Event actualEvent1 = f1.get();
    t1.join();
    EXPECT_EQ(actualEvent1.id, e1.id);
}

TEST_F(TestEventQueue, testAddFrom100Threads)
{
    EventQueue<Event> eq_;
    std::vector<Event> v;
    const int NEVENTS = 100;
    v.reserve(NEVENTS);

    for (int i = 0; i < NEVENTS; i++) {
        v.emplace_back();
    }

    std::vector<std::thread> vtProduce;
    std::vector<std::future<const Event>> vtConsume;

    for (auto event : v) {
        vtConsume.push_back(std::async(&EventQueue<Event>::nextEvent, &eq_));
        vtProduce.emplace_back(&EventQueue<Event>::addEvent, &eq_, event);
    }

    for (auto&& future : vtConsume) {
        const Event e = future.get();
        ASSERT_TRUE(std::find(v.begin(), v.end(), e) != v.end());
    }

    for (auto&& t : vtProduce) {
        t.join();
    }
}

int
main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
