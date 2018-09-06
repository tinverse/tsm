#pragma once
#include <glog/logging.h>
#include <inttypes.h>
#include <iostream>
#include <stdint.h>
#include <string>

#include "UniqueId.h"

namespace tsm {

struct State
{
  public:
    State() = delete;

    State(std::string stateName)
      : id(UniqueId::getId())
      , name(std::move(stateName))
    {}

    State(State const& other)
      : id(other.id)
      , name(other.name)
    {}

    State(State const&& other)
      : id(std::move(other.id))
      , name(std::move(other.name))
    {}

    virtual ~State() = default;

    bool operator==(State const& rhs) { return this->id == rhs.id; }

    // Methods
    virtual void execute()
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

    const std::string name;

  private:
    const uint64_t id;
};

} // namespace tsm
