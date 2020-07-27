#pragma once
#include <cstdint>

namespace tsm {
auto counter_inc = []() {
    thread_local uint64_t a = 0;
    return ++a;
};

} // namespace tsm
