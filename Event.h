#pragma once
#include "UniqueId.h"

#include <cstdint>
namespace tsm {

class Event
{
  public:
    uint64_t id;
    Event()
      : id(UniqueId::getId())
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

    static Event const
      dummy_event; ///< For startSM and stopSM calls, the state machine
                   ///< "automatically" transitions to the starting state.
                   ///< However, the State interface requires that an event be
                   ///< passed to the onEntry and onExit
};

} // namespace tsm
