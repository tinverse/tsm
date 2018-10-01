#pragma once

#include "Event.h"
#include "EventQueue.h"
#include "OrthogonalStateMachine.h"
#include "State.h"
#include "StateMachine.h"
#include "StateMachineDef.h"
#include "Transition.h"

#include "AsyncExecutionPolicy.h"
#include "ExecutionPolicy.h"
#include "ParentThreadExecutionPolicy.h"
#include "StateMachineWithExecutionPolicy.h"

namespace tsm {

///
/// A "simple" state machine. It executes in the context of the parent thread.
/// The user is expected to instantiate this with their HSM definition class.
/// Use it like this:
/// 1. The first step is to create your own state machine definition.
/// struct MyHSMDef : StateMachineDef<MyHSMDef> { ... };
/// 2. Then create your statemachine class by wrapping it arount the
/// StateMachine generic defined in "tsm.h". The StateMachine generic is a mixin
/// that mixes the ParentThreadExecutionPolicy along with the HSMBehavior
/// class.
/// using MyStateMachine = StateMachine<MyHSMDef>
/// 3. Now instantiate an object of your state machine type.
/// MyStateMachine mySMObj;
/// 4. Send events to it using the sendEvent method.
/// mySMObj.sendEvent(mySMObj.some_event);
/// 5. To process the event, call the step method
/// mySMObj.step();
///
template<typename HSMDef>
using StateMachine =
  StateMachineWithExecutionPolicy<HSMBehavior<HSMDef>,
                                  ParentThreadExecutionPolicy>;
///
/// An Asynchronous state machine. Event processing is done in a separate
/// thread. Usage is similar to the "simple" state machine above. The final call
/// to "step" is not required. The state machine is blocked waiting on the next
/// event to be processed. As soon as an event is sent to it (sendEvent), the
/// client/parent thread returns and the event is processed in a separate
/// thread. This guarantees that events are immediately processed as soon as
/// possible. It also simplifies the interface in that only one call to
/// sendEvent is required.
///
template<typename HSMDef>
using AsyncStateMachine =
  StateMachineWithExecutionPolicy<HSMBehavior<HSMDef>, AsyncExecutionPolicy>;
}

// Provide a hash function for StateEventPair
namespace std {
template<>
struct hash<tsm::StateEventPair>
{
    size_t operator()(const tsm::StateEventPair& s) const
    {
        shared_ptr<tsm::State> statePtr = s.first;
        uint64_t id_s = statePtr->id;
        tsm::Event event = s.second;
        auto address = reinterpret_cast<uintptr_t>(statePtr.get());
        uint64_t id_e = event.id;
        size_t hash_value = hash<uint64_t>{}(id_s) ^
                            hash<uintptr_t>{}(address) ^
                            (hash<uint64_t>{}(id_e) << 1);
        return hash_value;
    }
};
} // namespace std
