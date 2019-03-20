#pragma once

#include "AsyncExecutionPolicy.h"
#include "Event.h"
#include "EventQueue.h"
#include "FsmDefinition.h"
#include "FsmExecutor.h"
#include "HsmDefinition.h"
#include "HsmExecutor.h"
#include "OrthogonalHsmExecutor.h"
#include "SingleThreadedExecutionPolicy.h"
#include "State.h"
#include "Transition.h"

namespace tsm {

///
/// A "simple" state machine. It executes in the context of the parent thread.
/// The user is expected to instantiate this with their Hsm definition class.
/// Use it like this:
/// 1. The first step is to create your own state machine definition.
/// struct MyHsmDef : HsmDefinition<MyHsmDef> { ... };
/// 2. To create any statemachine, wrap an execution policy around the
/// HsmDefinition generic defined in HsmExecutor.h. Here, SingleThreadedHsm
/// is a mixin that combines the SingleThreadedExecutionPolicy along with the
/// HsmDefinition class; which in turn takes a state machine definition(HsmDef)
/// as seen in the using statement below.
/// 3. Now that you have your own SingleThreadedHsm, instantiate an object of
/// your state machine type. SingleThreadedHsm<MyHsmDef> sm;
/// 4. Send events to it using the sendEvent method.
/// sm.sendEvent(sm.some_event);
/// 5. To process the event, call the step method
/// sm.step();
///
template<typename HsmDef>
using SingleThreadedHsm = SingleThreadedExecutionPolicy<HsmExecutor<HsmDef>>;

template<typename FsmDef>
using SingleThreadedFsm = SingleThreadedExecutionPolicy<FsmExecutor<FsmDef>>;

///
/// An Asynchronous state machine. Event processing is done in a separate
/// thread. Usage is similar to SingleThreadedHsm above. The final call
/// to "step" is not required. The state machine is blocked waiting on the next
/// event to be processed. As soon as an event is sent to it (sendEvent), the
/// client/parent thread returns and the event is processed in a separate
/// thread. This design forces events to be immediately processed as soon as
/// the HsmDefinition is done processing the previous event. It also simplifies
/// the interface in that only one call to sendEvent is required.
///
template<typename HsmDef>
using AsynchronousHsm = AsyncExecutionPolicy<HsmExecutor<HsmDef>>;

template<typename FsmDef>
using AsynchronousFsm = AsyncExecutionPolicy<FsmExecutor<FsmDef>>;

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
