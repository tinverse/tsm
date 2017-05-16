#pragma once
#include <cstdint>
class Event
{
public:
    uint32_t id;
    Event()
        : id(0)
    {
    }
    Event(uint32_t id)
        : id(id)
    {
    }
    virtual ~Event() {}
    bool operator==(const Event& rhs) const { return this->id == rhs.id; }
    bool operator<(const Event& rhs) const { return this->id < rhs.id; }
};
