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
        , name(std::move(stateName))
    {
        LOG(INFO) << __PRETTY_FUNCTION__ << std::endl;
    }

    State(State const & other)
        :id(other.id),
        name(other.name) {
    }

    State(State const && other)
        : id(std::move(other.id)),
        name(std::move(other.name)) { }

    virtual ~State() { LOG(INFO) << __PRETTY_FUNCTION__ << std::endl; }

    bool operator==(State const & rhs)
    {
        return this->id == rhs.id;
    }

    // Methods
    virtual void execute() { LOG(WARNING) << this->name << std::endl; }
    virtual void OnEntry() { LOG(INFO) << __PRETTY_FUNCTION__ << std::endl; }
    virtual void OnExit() { LOG(INFO) << __PRETTY_FUNCTION__ << std::endl; }

    const std::string name;
private:
    // TODO: (sriram) make private
    const uint64_t id;


};
