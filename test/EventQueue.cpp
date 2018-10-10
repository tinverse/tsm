#include "EventQueue.h"
#include "Event.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <future>

using tsm::Event;
using tsm::EventQueue;

struct TestEventQueue : public testing::Test
{
    TestEventQueue()
      : testing::Test()
    {}

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
    std::vector<Event const> v;
    const int NEVENTS = 100;

    for (int i = 0; i < NEVENTS; i++) {
        v.emplace_back();
    }

    std::vector<std::thread> vtProduce;
    std::vector<std::future<const Event>> vtConsume;

    for (Event const& e : v) {
        auto f = std::async(&EventQueue<Event>::nextEvent, &eq_);
        // add a 100 threads to asynchronously wait on nextEvent
        vtConsume.push_back(std::move(f));
        // add 100 events to the event queue.
        vtProduce.emplace_back(&EventQueue<Event>::addEvent, &eq_, e);
    }

    for (auto&& future : vtConsume) {
        Event const& e = future.get();
        ASSERT_TRUE(std::find(v.begin(), v.end(), e) != v.end()) << e.id;
    }

    for (auto&& t : vtProduce) {
        t.join();
    }
}
