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
      , id(UniqueId::getId())
    {}

    State(State const& other) = default;

    State(State&& other) = default;

    virtual ~State() = default;

    bool operator==(State const& rhs) { return this->id == rhs.id; }

    virtual void execute(Event const& nextEvent)
    {

        LOG(INFO) << "Executing: " << this->name << std::endl;
    }

    virtual void onEntry(Event const&)
    {
        DLOG(INFO) << "Entering: " << this->name << std::endl;
    }

    virtual void onExit(Event const&)
    {
        DLOG(INFO) << "Exiting: " << this->name << std::endl;
    }

    virtual State* getParent() const { return nullptr; }

    virtual std::shared_ptr<State> const getCurrentState() const
    {
        throw MethodNotImplementedException(
          "The State::getCurrentState method is not implemented - by design. "
          "You should invoke "
          "StateMachine<DerivedHSM>::getCurrentState");
    }

    const std::string name;

    const uint64_t id;
};
} // namespace tsm
