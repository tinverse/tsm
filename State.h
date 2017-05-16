#pragma once

#include <inttypes.h>
#include <iostream>
#include <stdint.h>
// Generate and maintain an internal id
class State
{
public:
    State()
        : id(0)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    State(uint32_t id)
        : id(id)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    virtual ~State() { std::cout << __PRETTY_FUNCTION__ << std::endl; }

    // TODO: (sriram) make private
    const uint32_t id;

    bool operator==(const State& rhs) { return this == &rhs; }

    // Methods
    virtual void execute() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
    virtual void OnEntry() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
    virtual void OnExit() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
};


