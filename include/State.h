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
    State()
      : id(tsm::counter_inc())
    {}

    State(State const& other) = default;
    State(State&& other) = default;
    virtual ~State() = default;
    bool operator==(State const& rhs) { return this->id == rhs.id; }
    bool operator!=(State& rhs) { return !(*this == rhs); }

    virtual void execute(Event const&)
    {
        DLOG(INFO) << "Executing: " << this->id << std::endl;
    }

    virtual void onEntry(Event const&)
    {
        DLOG(INFO) << "Entering: " << this->id << std::endl;
    }

    virtual void onExit(Event const&)
    {
        DLOG(INFO) << "Exiting: " << this->id << std::endl;
    }

    std::ostream& operator<<(std::ostream& os)
    {
        os << this->id;
        return os;
    }

    const uint64_t id;
};

struct NamedState : public State
{
    NamedState() = delete;

    explicit NamedState(std::string const& stateName)
      : State()
      , name(stateName)
    {}

    NamedState(NamedState const& other) = default;
    NamedState(NamedState&& other) = default;
    virtual ~NamedState() = default;
    bool operator==(NamedState const& rhs)
    {
        return this->id == rhs.id && this->name == rhs.name;
    }
    bool operator!=(NamedState& rhs) { return !(*this == rhs); }

    void execute(Event const&) override
    {
        DLOG(INFO) << "Executing: " << this->name << std::endl;
    }

    void onEntry(Event const&) override
    {
        DLOG(INFO) << "Entering: " << this->name << std::endl;
    }

    void onExit(Event const&) override
    {
        DLOG(INFO) << "Exiting: " << this->name << std::endl;
    }

    std::ostream& operator<<(std::ostream& os)
    {
        os << this->name;
        return os;
    }

    const std::string name;
};

} // namespace tsm
