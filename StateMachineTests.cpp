#include <future>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <set>
#include "Event.h"
#include "EventQueue.h"
#include "State.h"
#include "StateMachine.h"

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

// TODO: Test no end state
// Model interruptions to workflow
// For e.g. As door is opening or closing, one of the sensors
// detects an obstacle.

class TestStateMachine : public testing::Test
{

};
TEST_F(TestStateMachine, testTransition)
{
    // States
    auto doorOpen    = std::make_shared<State>(10, "Door Open");
    auto doorOpening = std::make_shared<State>(15, "Door Opening");
    auto doorClosed  = std::make_shared<State>(20, "Door Closed");
    auto doorClosing = std::make_shared<State>(25, "Door Closing");

    // Events
    Event close_event(0);
    Event bottomSensor_event(1);
    Event topSensor_event(2);
    Event open_event(3);



    // Transitions
    Transition openTransition(doorClosed, open_event, doorOpening);
    Transition openedTransition(doorOpening, topSensor_event, doorOpen);
    Transition closeTransition(doorOpen, close_event, doorClosing);
    Transition closedTransition(doorClosing, bottomSensor_event, doorClosed);

    // Add Transitions to the table

    // TransitionTable
    StateTransitionTable garageDoorTransitions;

    garageDoorTransitions.add(openTransition);
    garageDoorTransitions.add(openedTransition);
    garageDoorTransitions.add(closeTransition);
    garageDoorTransitions.add(closedTransition);

    // Add the open event
    EventQueue<Event> garageDoorEventQueue;

    // The StateMachine
    // Starting State: doorClosed
    // Stop State: doorOpen
    StateMachine garageDoorStateMachine(
        doorClosed , doorOpen, garageDoorEventQueue, garageDoorTransitions);

    garageDoorStateMachine.start();

    EXPECT_EQ(garageDoorStateMachine.getCurrentState()->id, doorClosed->id);
    garageDoorEventQueue.addEvent(open_event);

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    EXPECT_EQ(doorOpening->id, garageDoorStateMachine.getCurrentState()->id);
    garageDoorEventQueue.addEvent(topSensor_event);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    EXPECT_EQ(garageDoorStateMachine.getCurrentState()->id, doorOpen->id);


    garageDoorStateMachine.stop();

}

int main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
