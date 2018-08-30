#pragma once
#include <cstdint>

#include "UniqueId.h"
namespace tsm {

class Event
{
  public:
    uint32_t id;
    Event()
      : id(UniqueId::getId())
    {}
    virtual ~Event() = default;
    bool operator==(const Event& rhs) const { return this->id == rhs.id; }
    bool operator<(const Event& rhs) const { return this->id < rhs.id; }
};

} // namespace tsm
