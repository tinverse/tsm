#include "tsm.h"

using tsm::Event;
using tsm::EventQueue;
using tsm::State;
using tsm::HsmDefinition;

namespace tsmtest {
struct GarageDoorDef : public HsmDefinition<GarageDoorDef>
{
    GarageDoorDef(IHsmDef* parent = nullptr)
      : HsmDefinition<GarageDoorDef>("Garage Door Hsm", parent)
      , doorOpen("Door Open")
      , doorOpening("Door Opening")
      , doorClosing("Door Closing")
      , doorClosed("Door Closed")
      , doorStoppedClosing("Door Stopped Closing")
      , doorStoppedOpening("Door Stopped Opening")
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

    State* getStartState() override { return &doorClosed; }
    State* getStopState() override { return nullptr; }

    // States
    State doorOpen;
    State doorOpening;
    State doorClosing;
    State doorClosed;
    State doorStoppedClosing;
    State doorStoppedOpening;

    // Events
    Event click_event;
    Event bottomSensor_event;
    Event topSensor_event;
    Event obstruct_event;
};
} // namespace tsmtest
