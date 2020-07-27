#pragma once
#include "UniqueId.h"

#include <cstdint>
namespace tsm {

struct Event
{
    Event()
      : id(tsm::counter_inc())
    {}

    Event(uint64_t id_)
      : id(id_)
    {}

    Event(Event const& other) = default;
    Event(Event&& other) = default;

    Event& operator=(Event const& e)
    {
        if (this != &e) {
            this->id = e.id;
        }
        return *this;
    }

    virtual ~Event() = default;

    bool operator==(const Event& rhs) const { return this->id == rhs.id; }
    bool operator!=(const Event& rhs) const { return this->id != rhs.id; }
    bool operator<(const Event& rhs) const { return this->id < rhs.id; }

    ///< For startSM and stopSM calls, the state machine
    ///< "automatically" transitions to the starting state.
    ///< However, the State interface requires that an event
    ///< be passed to the onEntry and onExit

    uint64_t id;
};

static Event const null_event = Event{};
} // namespace tsm
