#pragma once

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

    State(uint32_t id)
        : id(id)
    {
        LOG(INFO) << __PRETTY_FUNCTION__ << std::endl;
    }

    virtual ~State() { LOG(INFO) << __PRETTY_FUNCTION__ << std::endl; }

    // TODO: (sriram) make private
    const uint32_t id;

    // Methods
    virtual void execute() { LOG(INFO) << __PRETTY_FUNCTION__ << std::endl; }
    virtual void OnEntry() { LOG(INFO) << __PRETTY_FUNCTION__ << std::endl; }
    virtual void OnExit() { LOG(INFO) << __PRETTY_FUNCTION__ << std::endl; }
};
