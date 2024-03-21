#pragma once

#include "hsm.h"

namespace tsm {

///
/// A "simple" state machine. It executes in the context of the parent thread.
/// The user is expected to instantiate this with their state machine context.
/// Use it like this:
/// 1. The first step is to create your own Trait type.
/// struct MyTraits {
///    // States (structs)
///    struct State1 {};
///    struct State2 {};
///    // Events (structs)
///    struct Event1 {};
///    // e.g. transition where
///    using transitions = std::tuple<Transition<State1, Event1, State2>>;
/// };
/// 2. To create any statemachine, wrap an execution policy around the
/// trait type. Here we use the SingleThreadedExecutionPolicy.
/// 3. Now that you have your own SingleThreadedHsm, instantiate an object of
/// your state machine type. SingleThreadedHsm<MyTraits> sm;
/// 4. Send events to it using the sendEvent method.
/// sm.send_event(MyTraits::Event1{});
/// 5. To process the event, call the step method
/// sm.step();
///
template<typename Context>
using SingleThreadedHsm = SingleThreadedExecutionPolicy<Context>;

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
template<typename Context>
using ThreadedHsm = ThreadedExecutionPolicy<Context>;

// This state machine is driven by a periodic timer.
template<typename Context,
         template<class, class>
         class TimerType,
         typename ClockType,
         typename DurationType>
using PeriodicHsm =
  PeriodicExecutionPolicy<Context, TimerType<ClockType, DurationType>>;

// This real-time state machine is driven by a periodic timer.
template<typename Context,
         template<class, class>
         class TimerType,
         typename ClockType,
         typename DurationType>
using RealtimePeriodicHsm =
  PeriodicExecutionPolicy<Context, TimerType<ClockType, DurationType>>;

// Real-time state machine. This state machine is driven by a periodic timer.
template<typename Context>
using RealtimeHsm = RealtimeExecutionPolicy<Context>;

// Concurrent Hsm
template<typename Policy = ThreadedExecutionPolicy, typename... Contexts>
using ConcurrentHsm = make_concurrent_hsm_t<Policy, Contexts...>;

} // namespace tsm

} // namespace tsm
