#pragma once

#include "AsyncExecutionPolicy.h"
#include "Event.h"
#include "EventQueue.h"
#include "Hsm.h"
#include "OrthogonalHsm.h"
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
template<typename Hsm>
using SingleThreadedHsm = SingleThreadedExecutionPolicy<Hsm>;

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
template<typename Hsm>
using AsynchronousHsm = AsyncExecutionPolicy<Hsm>;

} // namespace tsm
