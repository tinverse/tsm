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
    Event(uint32_t id_)
        : id(id_)
    {
    }
    virtual ~Event() = default;
    bool operator==(const Event& rhs) const { return this->id == rhs.id; }
    bool operator<(const Event& rhs) const { return this->id < rhs.id; }
};
