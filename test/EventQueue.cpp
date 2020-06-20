#include "EventQueue.h"
#include "Event.h"

#include <catch2/catch.hpp>
#include <future>

using tsm::Event;
using tsm::EventQueue;

TEST_CASE("TestEventQueue - testSingleEvent")
{
    EventQueue<Event> eq_;
    Event e1;
    auto f1 = std::async(&EventQueue<Event>::nextEvent, &eq_);

    std::thread t1(&EventQueue<Event>::addEvent, &eq_, e1);

    // Use the same threads to retrieve events
    Event actualEvent1 = f1.get();
    t1.join();
    CHECK(actualEvent1.id == e1.id);
}

TEST_CASE("TestEventQueue - testAddFrom100Threads")
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

    for (Event const& e : v) {
        auto f = std::async(&EventQueue<Event>::nextEvent, &eq_);
        // add a 100 threads to asynchronously wait on nextEvent
        vtConsume.push_back(std::move(f));
        // add 100 events to the event queue.
        vtProduce.emplace_back(&EventQueue<Event>::addEvent, &eq_, e);
    }

    for (auto&& future : vtConsume) {
        Event const& e = future.get();
        auto it = std::find(v.begin(), v.end(), e);
        CHECK(it != v.end());
    }

    for (auto&& t : vtProduce) {
        t.join();
    }
}
