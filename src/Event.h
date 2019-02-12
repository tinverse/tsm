#pragma once
#include "UniqueId.h"

#include <cstdint>
namespace tsm {

class Event
{
  public:
    Event()
      : id(tsm::counter_inc()())
    {}

    Event(Event const& other) = default;

    Event(Event&& other) = default;

    Event& operator=(Event const& e)
    {
        this->id = e.id;
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

Event const dummy_event = Event{};
} // namespace tsm
