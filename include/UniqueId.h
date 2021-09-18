#pragma once
#include <cstdint>

namespace tsm {
using id_t = uint16_t;
struct Counter
{
    static id_t counter_inc()
    {
        thread_local id_t a = 0;
        return ++a;
    }
};

} // namespace tsm
