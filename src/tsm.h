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

namespace tsm {

///
/// A "simple" state machine. It executes in the context of the parent thread.
/// The user is expected to instantiate this with their HSM definition class.
/// Use it like this:
/// 1. The first step is to create your own state machine definition.
/// struct MyHSMDef : StateMachineDef<MyHSMDef> { ... };
/// 2. To create any statemachine, wrap an execution policy around the
/// StateMachine generic defined in StateMachine.h. Here, SimpleStateMachine
/// is a mixin that combines the ParentThreadExecutionPolicy along with the
/// StateMachine class; which in turn takes a state machine definition(HSMDef)
/// as seen in the using statement below.
/// 3. Now that you have your own SimpleStateMachine, instantiate an object of
/// your state machine type. SimpleStateMachine<MyHSMDef> sm;
/// 4. Send events to it using the sendEvent method.
/// sm.sendEvent(sm.some_event);
/// 5. To process the event, call the step method
/// sm.step();
///
template<typename HSMDef>
using SimpleStateMachine = ParentThreadExecutionPolicy<StateMachine<HSMDef>>;
///
/// An Asynchronous state machine. Event processing is done in a separate
/// thread. Usage is similar to SimpleStateMachine above. The final call
/// to "step" is not required. The state machine is blocked waiting on the next
/// event to be processed. As soon as an event is sent to it (sendEvent), the
/// client/parent thread returns and the event is processed in a separate
/// thread. This design forces events to be immediately processed as soon as
/// the StateMachine is done processing the previous event. It also simplifies
/// the interface in that only one call to sendEvent is required.
///
template<typename HSMDef>
using AsyncStateMachine = AsyncExecutionPolicy<StateMachine<HSMDef>>;
}

// Provide a hash function for StateEventPair
namespace std {
using tsm::State;
template<>
struct hash<tsm::StateEventPair>
{
    size_t operator()(const tsm::StateEventPair& s) const
    {
        State* statePtr = &s.first;
        uint64_t id_s = statePtr->id;
        tsm::Event event = s.second;
        auto address = reinterpret_cast<uintptr_t>(statePtr);
        uint64_t id_e = event.id;
        size_t hash_value = hash<uint64_t>{}(id_s) ^
                            hash<uintptr_t>{}(address) ^
                            (hash<uint64_t>{}(id_e) << 1);
        return hash_value;
    }
};
} // namespace std
