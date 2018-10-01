#pragma once
#include <cstdint>
#include <mutex>

namespace tsm {

struct UniqueId
{
    static uint64_t getId() { return ++id_; }
    static void reset()
    {
        id_ = 1; // dummy_event gets id=0 assigned to it
    }

  private:
    static std::uint64_t id_;
};

} // namespace tsm
