#include <glog/logging.h>
#include <gtest/gtest.h>

#include "StateMachineTest.h"

namespace tsmtest {
struct GarageDoorSM : public StateMachine<GarageDoorSM>
{
    GarageDoorSM() = delete;

    GarageDoorSM(std::string name, EventQueue<Event>& eventQueue)
      : StateMachine<GarageDoorSM>(name, eventQueue)
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
} // namespace tsmtest
