#pragma once
#include <inttypes.h>
#include <iostream>
#include <stdint.h>
#include <string>

#include <glog/logging.h>

#include "UniqueId.h"

// Generate and maintain an internal id
struct State
{
public:
    State() = delete;

    State(std::string stateName)
        : id(UniqueId::getId())
        , name(std::move(stateName)) { }

    State(State const & other)
        :id(other.id),
        name(other.name) {
    }

    State(State const && other)
        : id(std::move(other.id)),
        name(std::move(other.name)) { }

    virtual ~State() = default;

    bool operator==(State const & rhs)
    {
        return this->id == rhs.id;
    }

    // Methods
    virtual void execute() { LOG(INFO) << "Executing: " << this->name << std::endl; }
    virtual void OnEntry() { DLOG(INFO) << "Entering: " << this->name << std::endl; }
    virtual void OnExit() { DLOG(INFO) << "Exiting: "  << this->name << std::endl; }

    const std::string name;
private:
    // TODO: (sriram) make private
    const uint64_t id;


};
