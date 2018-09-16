#pragma once

#include <glog/logging.h>
#include <inttypes.h>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <string>

#include "Event.h"
#include "UniqueId.h"

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

struct State
{
  public:
    State() = delete;

    State(std::string stateName)
      : name(std::move(stateName))
      , id(UniqueId::getId())
    {}

    State(State const& other) = default;

    State(State&& other) = default;

    virtual ~State() = default;

    bool operator==(State const& rhs) { return this->id == rhs.id; }

    // Methods
    virtual void execute()
    {
        LOG(INFO) << "Executing: " << this->name << std::endl;
    }

    virtual void execute(Event const& nextEvent)
    {

        LOG(INFO) << "Executing: " << this->name << std::endl;
    }

    virtual void OnEntry()
    {
        DLOG(INFO) << "Entering: " << this->name << std::endl;
    }

    virtual void OnExit()
    {
        DLOG(INFO) << "Exiting: " << this->name << std::endl;
    }

    virtual State* getParent() const { return nullptr; }

    virtual std::shared_ptr<State> const getCurrentState() const
    {
        throw MethodNotImplementedException(
          "The State::getCurrentState method is not implemented. You "
          "probably meant to call "
          "StateMachine<DerivedHSM>::getCurrentState");
    }
    const std::string name;

  private:
    const uint64_t id;
};
} // namespace tsm
