#include "Event.h"
#include "EventQueue.h"
#include "State.h"

#include <future>
#include <gtest/gtest.h>
#include <set>

class TestState : public testing::Test
{
public:
    virtual ~TestState() {}
    void SetUp() {}
    void TearDown() {}

protected:
    State state_;
};

TEST_F(TestState, Construct) { std::cout << "Test" << std::endl; }

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

TEST_F(TestEventQueue, testTwoEvents)
{
    auto f1 = std::async(std::launch::async, &EventQueue<Event>::nextEvent, &eq_);
    auto f2 = std::async(std::launch::async, &EventQueue<Event>::nextEvent, &eq_);
    std::thread f3(&EventQueue<Event>::nextEvent, &eq_);

    // Add 3 events from 3 threads
    std::thread t1(&EventQueue<Event>::addEvent, &eq_, e1);
    std::thread t2(&EventQueue<Event>::addEvent, &eq_, e2);
    std::thread t3(&EventQueue<Event>::addEvent, &eq_, e3);
    std::set<Event> eventSet;
    // Use the same threads to retrieve events
    t1.join();
    t2.join();
    t3.join();
    f3.join();
    Event ae1 = f1.get();
    eventSet.insert(ae1); //, ae2 = f2.get();//, ae3=f3.get();
    // std::printf("Event:%d ", ae1.id);
    eventSet.insert(f2.get()); //, ae2 = f2.get();//, ae3=f3.get();
    // std::printf("Event:%d ", ae2.id);
    std::set<Event>::iterator it = eventSet.begin();
    EXPECT_TRUE(it != eventSet.end());
    it = eventSet.begin();
    it = eventSet.find(e2.id);
    EXPECT_TRUE(it != eventSet.end());
}
