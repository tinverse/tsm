#pragma once
#include <cstdint>

namespace tsm {
struct Counter {
    static uint64_t counter_inc() {
        thread_local uint64_t a = 0;
        return ++a;
    }
};

} // namespace tsm
