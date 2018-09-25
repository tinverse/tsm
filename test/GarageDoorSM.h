#include "tsm.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

using tsm::Event;
using tsm::EventQueue;
using tsm::ParentThreadExecutionPolicy;
using tsm::AsyncExecutionPolicy;
using tsm::State;
using tsm::StateMachine;
using tsm::StateMachineDef;
using tsm::StateMachineExecutionPolicy;

namespace tsmtest {
struct GarageDoorDef : public StateMachineDef<GarageDoorDef>
{
    // Start State declared first
    static const shared_ptr<State> doorClosed;

    GarageDoorDef(State* parent = nullptr)
      : StateMachineDef<GarageDoorDef>("Garage Door HSM",
                                       doorClosed,
                                       nullptr,
                                       parent)
      , doorOpen(std::make_shared<State>("Door Open"))
      , doorOpening(std::make_shared<State>("Door Opening"))
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

    virtual ~GarageDoorDef() = default;

    shared_ptr<State> getStartState() const { return doorClosed; }

    // States
    shared_ptr<State> doorOpen;
    shared_ptr<State> doorOpening;

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
