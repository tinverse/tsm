#include <glog/logging.h>
#include <gtest/gtest.h>

#include "StateMachineTest.h"

using tsm::Event;
using tsm::EventQueue;
using tsm::OrthogonalHSM;
using tsm::State;
using tsm::StateMachine;

using tsmtest::StateMachineTest;

struct GarageDoorSM : public StateMachineTest<GarageDoorSM>
{
    GarageDoorSM() = delete;

    GarageDoorSM(std::string name, EventQueue<Event>& eventQueue)
      : StateMachineTest<GarageDoorSM>(name, eventQueue)
      , doorOpen(std::make_shared<State>("Door Open"))
      , doorOpening(std::make_shared<State>("Door Opening"))
      , doorClosed(std::make_shared<State>("Door Closed"))
      , doorClosing(std::make_shared<State>("Door Closing"))
      , doorStoppedClosing(std::make_shared<State>("Door Stopped Closing"))
      , doorStoppedOpening(std::make_shared<State>("Door Stopped Opening"))
    {
        // TransitionTable
        add(doorClosed, click_event, doorOpening);
        add(doorOpening, topSensor_event, doorOpen);
        add(doorOpen, click_event, doorClosing);
        add(doorClosing, bottomSensor_event, doorClosed);
        add(doorOpening, click_event, doorStoppedOpening);
        add(doorStoppedOpening, click_event, doorClosing);
        add(doorClosing, obstruct_event, doorStoppedClosing);
        add(doorClosing, click_event, doorStoppedClosing);
        add(doorStoppedClosing, click_event, doorOpening);
        add(doorClosed, click_event, doorOpening);
    }

    virtual ~GarageDoorSM() = default;

    shared_ptr<State> getStartState() const override { return doorClosed; }

    // States
    shared_ptr<State> doorOpen;
    shared_ptr<State> doorOpening;
    shared_ptr<State> doorClosed;
    shared_ptr<State> doorClosing;
    shared_ptr<State> doorStoppedClosing;
    shared_ptr<State> doorStoppedOpening;

    // Events
    Event click_event;
    Event bottomSensor_event;
    Event topSensor_event;
    Event obstruct_event;
};

struct TestGarageDoorSM : public ::testing::Test
{
    TestGarageDoorSM()
      : testing::Test()
      , sm(std::make_shared<GarageDoorSM>("Garage Door SM", eventQueue))
    {}
    // Event Queue
    EventQueue<Event> eventQueue;

    // The StateMachine
    // Starting State: doorClosed
    // Stop State: doorOpen
    std::shared_ptr<GarageDoorSM> sm;
};

TEST_F(TestGarageDoorSM, testGarageDoor)
{
    sm->OnEntry();
    EXPECT_EQ(sm->getCurrentState(), sm->doorClosed);
    eventQueue.addEvent(sm->click_event);
    EXPECT_EQ(sm->doorOpening, sm->getCurrentState());
    eventQueue.addEvent(sm->topSensor_event);
    EXPECT_EQ(sm->doorOpen, sm->getCurrentState());
    sm->OnExit();
}
