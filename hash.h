#pragma once

namespace std {
template<>
struct hash<StateEventPair>
{
    size_t operator()(const StateEventPair& s) const
    {
        std::shared_ptr<State> statePtr = s.first;
        Event event = s.second;
        size_t address = reinterpret_cast<size_t>(statePtr.get());
        size_t id = event.id;
        size_t hash_value =
          std::hash<int>{}(address) ^ (std::hash<size_t>{}(id) << 1);
        return hash_value;
    }
};
} // namespace std
