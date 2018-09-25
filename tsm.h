#pragma once

#include "Event.h"
#include "EventQueue.h"
#include "OrthogonalStateMachine.h"
#include "State.h"
#include "StateMachine.h"
#include "StateMachineDef.h"
#include "Transition.h"

#include "AsyncExecutionPolicy.h"
#include "ParentThreadExecutionPolicy.h"
#include "ExecutionPolicy.h"
#include "StateMachineExecutionPolicy.h"

#include <memory>

using std::shared_ptr;

// Provide a hash function for StateEventPair
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
