#pragma once
#include <cstdint>
#include <mutex>

namespace tsm {
constexpr auto
counter_inc()
{
    return []() {
        static uint64_t a = 0;
        return ++a;
    };
}

} // namespace tsm
