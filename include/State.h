#pragma once

#include "Event.h"
#include "UniqueId.h"
#include "tsm_log.h"

#include <memory>
#include <string>

namespace tsm {

///
/// All HsmDefinition types inherit from State. This is the base class for all
/// StateMachines.
///
struct State
{
    State() = delete;

    explicit State(std::string const& stateName)
      : name(stateName)
      , id(tsm::counter_inc())
    {}

    State(State const& other) = default;
    State(State&& other) = default;
    virtual ~State() = default;
    bool operator==(State const& rhs) { return this->id == rhs.id; }
    bool operator!=(State& rhs) { return !(*this == rhs); }

    // Implement a transition for a moore machine.
    // Given the current "input/event", what's the next?
    virtual State* execute(Event const&)
    {
        DLOG(INFO) << "Executing: " << this->name << std::endl;
        // return the next state
        return this;
    }

    virtual void onEntry(Event const&)
    {
        DLOG(INFO) << "Entering: " << this->name << std::endl;
    }

    virtual void onExit(Event const&)
    {
        DLOG(INFO) << "Exiting: " << this->name << std::endl;
    }

    std::ostream& operator<<(std::ostream& os)
    {
        os << "State: " << this->name;
        return os;
    }

    const std::string name;
    const uint64_t id;
};

} // namespace tsm
