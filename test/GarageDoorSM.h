#include "tsm.h"

namespace tsmtest {

using tsm::Event;
using tsm::EventQueue;
using tsm::Hsm;
using tsm::State;

struct GarageDoorHsm : public Hsm<GarageDoorHsm>
{
    GarageDoorHsm()
    {
        // Start State
        setStartState(&doorClosed);

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

    virtual ~GarageDoorHsm() = default;

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
