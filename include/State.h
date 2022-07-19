#pragma once

#include "Event.h"
#include "UniqueId.h"
#include "tsm_log.h"

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

    virtual void execute(Event const& /*e*/)
    {
        LOG(INFO) << "Executing: " << this->id;
    }

    virtual void onEntry(Event const& /*unused*/)
    {
        LOG(INFO) << "Entering: " << this->id;
    }

    virtual void onExit(Event const& /*unused*/)
    {
        LOG(INFO) << "Exiting: " << this->id;
    }
    const id_t id;
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
        LOG(INFO) << "Executing: " << this->name;
    }

    void onEntry(Event const& /*unused*/) override
    {
        LOG(INFO) << "Entering: " << this->name;
    }

    void onExit(Event const& /*unused*/) override
    {
        LOG(INFO) << "Exiting: " << this->name;
    }

    const std::string name;
};

} // namespace tsm
