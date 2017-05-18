#pragma once
#include <string>
#include <inttypes.h>
#include <iostream>
#include <stdint.h>

#include <glog/logging.h>

// Generate and maintain an internal id
class State
{
public:
    State()
        : id(0)
    {
        LOG(INFO) << __PRETTY_FUNCTION__ << std::endl;
    }

    State(uint32_t id, std::string stateName)
        : id(id)
        , name(stateName)
    {
        LOG(INFO) << __PRETTY_FUNCTION__ << std::endl;
    }

    virtual ~State() { LOG(INFO) << __PRETTY_FUNCTION__ << std::endl; }

    // TODO: (sriram) make private
    const uint32_t id;
    const std::string name;

    // Methods
    virtual void execute() { LOG(WARNING) << this->name << std::endl; }
    virtual void OnEntry() { LOG(INFO) << __PRETTY_FUNCTION__ << std::endl; }
    virtual void OnExit() { LOG(INFO) << __PRETTY_FUNCTION__ << std::endl; }
};
