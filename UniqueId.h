#pragma once
#include <cstdint>
struct UniqueId
{
    static uint64_t getId()
    {
        return ++id_;
    }
private:
    static std::uint64_t id_;
};
