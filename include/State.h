#pragma once

#include "Event.h"
#include "UniqueId.h"
#include "tsm_log.h"

#include <iostream>
#include <string>

namespace tsm {

///
/// All HsmDefinition types inherit from State. This is the base class for all
/// StateMachines.
///
struct State
{
    State()
      : id(Counter::counter_inc())
    {}

    State(State const& other) = delete;
    State(State&& other) = delete;
    State& operator=(State const& other) = delete;
    State& operator=(State const&& other) = delete;
    virtual ~State() = default;
    bool operator==(State const& rhs) const { return this->id == rhs.id; }
    bool operator!=(State& rhs) const { return !(*this == rhs); }

    std::ostream& operator<<(std::ostream& os) const
    {
        os << this->id;
        return os;
    }


    virtual void execute(Event const&  /*e*/)
    {
        LOG(INFO) << "Executing: " << this->id << std::endl;
    }

    virtual void onEntry(Event const& /*unused*/)
    {
        LOG(INFO) << "Entering: " << this->id << std::endl;
    }

    virtual void onExit(Event const& /*unused*/)
    {
        LOG(INFO) << "Exiting: " << this->id << std::endl;
    }
    const uint64_t id;
};

struct NamedState : public State
{
    NamedState() = delete;

    explicit NamedState(std::string stateName)
      : name(std::move(stateName))
    {}
    NamedState(NamedState const& other) = delete;
    NamedState(NamedState&& other) = delete;
     ~NamedState() override = default;

    NamedState& operator=(NamedState const& other) = delete;
    NamedState& operator=(NamedState const&& other) = delete;
    bool operator==(NamedState const& rhs)
    {
        return this->id == rhs.id && this->name == rhs.name;
    }
    bool operator!=(NamedState& rhs) { return !(*this == rhs); }

    void execute(Event const& /*unused*/) override
    {
        LOG(INFO) << "Executing: " << this->name << std::endl;
    }

    void onEntry(Event const& /*unused*/) override
    {
        LOG(INFO) << "Entering: " << this->name << std::endl;
    }

    void onExit(Event const& /*unused*/) override
    {
        LOG(INFO) << "Exiting: " << this->name << std::endl;
    }

    std::ostream& operator<<(std::ostream& os)
    {
        os << this->name;
        return os;
    }

    const std::string name;
};

} // namespace tsm
