#pragma once
#include <cstdint>
#include <mutex>

namespace tsm {
auto counter_inc = []() {
    static uint64_t a = 0;
    return ++a;
};

} // namespace tsm
