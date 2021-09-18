#pragma once
#include "UniqueId.h"

#include <cstdint>
namespace tsm {

struct Event
{
    Event()
      : id(Counter::counter_inc())
    {}

    bool operator==(const Event& rhs) const { return this->id == rhs.id; }
    bool operator!=(const Event& rhs) const { return !(*this == rhs); }
    bool operator<(const Event& rhs) const { return this->id < rhs.id; }

    id_t id;
};

///< For startSM and stopSM calls, the state machine
///< "automatically" transitions to the starting state.
///< However, the State interface requires that an event
///< be passed to the onEntry and onExit. Hence "null_event"

static Event const null_event = Event{};
} // namespace tsm
