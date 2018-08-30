#pragma once
#include <cstdint>

namespace tsm {

struct UniqueId
{
    static uint64_t getId() { return ++id_; }

  private:
    static std::uint64_t id_;
};

} // namespace tsm
