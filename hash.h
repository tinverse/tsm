#pragma once
#include "Event.h"
#include "State.h"

#include <memory>
#include <utility>

namespace tsm {
typedef std::pair<std::shared_ptr<State>, Event> StateEventPair;
}

namespace std {
template<>
struct hash<tsm::StateEventPair>
{
    size_t operator()(const tsm::StateEventPair& s) const
    {
        shared_ptr<tsm::State> statePtr = s.first;
        tsm::Event event = s.second;
        size_t address = reinterpret_cast<size_t>(statePtr.get());
        size_t id = event.id;
        size_t hash_value = hash<int>{}(address) ^ (hash<size_t>{}(id) << 1);
        return hash_value;
    }
};

} // namespace std
