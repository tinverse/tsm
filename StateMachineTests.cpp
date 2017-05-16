#include <future>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <set>
#include "Event.h"
#include "EventQueue.h"
#include "State.h"

class TestState : public testing::Test
{
public:
    virtual ~TestState() {}
    void SetUp() {}
    void TearDown() {}

protected:
    State state_;
};

TEST_F(TestState, Construct) { DLOG(INFO) << "Test" << std::endl; }

TEST_F(TestState, Call) { state_.execute(); }

class TestEventQueue : public testing::Test
{
public:
    virtual ~TestEventQueue() {}
    void SetUp() {}
    void TearDown() {}

protected:
    EventQueue<Event> eq_;
    Event e1{1}, e2{2}, e3{3};
};

TEST_F(TestEventQueue, testSingleEvent)
{
    auto f1 = std::async(&EventQueue<Event>::nextEvent, &eq_);

    std::thread t1(&EventQueue<Event>::addEvent, &eq_, e1);

    // Use the same threads to retrieve events
    Event actualEvent1 = f1.get(); //, e2 = f2.get(), e3=f3.get();
    t1.join();                     // t2.join(); t3.join();
    // std::printf("Event1:%d Event2:%d Event3:%d", e1.id, e2.id, e3.id);
    // std::printf("Event1:%d", e1.id);
    EXPECT_EQ(actualEvent1.id, e1.id);
}

TEST_F(TestEventQueue, testAddFrom100Threads)
{

    std::vector<Event> v; //{e3, e1, e2};
    const int NEVENTS = 100;
    for (int i = 0; i < NEVENTS; i++)
    {
        v.push_back(Event(i));
    }

    std::vector<std::thread> vt;
    for (auto event : v)
    {
        vt.push_back(std::thread(&EventQueue<Event>::addEvent, &eq_, event));
    }

    for (auto& t : vt)
    {
        t.join();
    }

    EXPECT_EQ(eq_.size(), NEVENTS);

    while (!eq_.empty())
    {
        auto e = eq_.front();
        ASSERT_TRUE(std::find(v.begin(), v.end(), e) != v.end());
        eq_.pop();
    }
}

int main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
