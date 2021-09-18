#include "tsm.h"

namespace tsmtest {

using tsm::Event;
using tsm::Hsm;
using tsm::State;

struct GarageDoorHsm : public Hsm<GarageDoorHsm>
{
    GarageDoorHsm()
    {
        // Start State
        setStartState(&DoorClosed);

        // TransitionTable
        add(DoorClosed, click_event, DoorOpening);
        add(DoorOpening, sensor_hi_event, DoorOpen);
        add(DoorOpen, click_event, DoorClosing);
        add(DoorClosing, sensor_hi_event, DoorClosed);
        add(DoorOpening, click_event, DoorStoppedOpening);
        add(DoorStoppedOpening, click_event, DoorClosing);
        add(DoorClosing, obstruct_event, DoorStoppedClosing);
        add(DoorClosing, click_event, DoorStoppedClosing);
        add(DoorStoppedClosing, click_event, DoorOpening);
        add(DoorClosed, click_event, DoorOpening);
    }

    // States
    State DoorOpen, DoorOpening, DoorClosing, DoorClosed, DoorStoppedClosing,
      DoorStoppedOpening;

    // Events
    Event click_event, sensor_lo_event, sensor_hi_event, obstruct_event;
};
} // namespace tsmtest
