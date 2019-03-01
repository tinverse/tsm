#pragma once

#include "Event.h"
#include "UniqueId.h"

#include <glog/logging.h>

#include <memory>
#include <string>

namespace tsm {

struct MethodNotImplementedException : public std::runtime_error
{
    explicit MethodNotImplementedException(const std::string& what_arg)
      : std::runtime_error(what_arg)
    {}
    explicit MethodNotImplementedException(const char* what_arg)
      : std::runtime_error(what_arg)
    {}
};

///
/// All StateMachineDef types inherit from State. This is the base class for all
/// StateMachines.
///
struct State
{
  public:
    State() = delete;

    State(std::string const& stateName)
      : name(std::move(stateName))
      , id(tsm::counter_inc()())
    {}

    State(State const& other) = default;
    State(State&& other) = default;
    virtual ~State() = default;
    bool operator==(State const& rhs) { return this->id == rhs.id; }
    bool operator!=(State& rhs) { return !(*this == rhs); }

    virtual void execute(Event const& nextEvent)
    {

        DLOG(INFO) << "Executing: " << this->name << std::endl;
    }

    virtual void onEntry(Event const&)
    {
        DLOG(INFO) << "Entering: " << this->name << std::endl;
    }

    virtual void onExit(Event const&)
    {
        DLOG(INFO) << "Exiting: " << this->name << std::endl;
    }

    const std::string name;
    const uint64_t id;
};
} // namespace tsm
