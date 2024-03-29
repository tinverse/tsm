#pragma once

#include "tsm_impl.h"

namespace tsm {

///
/// A "simple" state machine. It executes in the context of the parent thread.
/// The user is expected to instantiate this with their state machine context.
/// Use it like this:
/// 1. The first step is to create your own Context type.
/// struct MyContext {
///    // States (structs)
///    struct State1 {};
///    struct State2 {};
///    // Events (structs)
///    struct Event1 {};
///    // e.g. transition where
///    using transitions = std::tuple<Transition<State1, Event1, State2>>;
/// };
/// 2. To create any statemachine, wrap an execution policy around the
/// context type. Here we use the SingleThreadedExecutionPolicy.
/// 3. Now that you have your own SingleThreadedHsm, instantiate an object of
/// your state machine type. SingleThreadedHsm<MyContext> sm;
/// 4. Send events to it using the sendEvent method.
/// sm.send_event(MyContext::Event1{});
/// 5. To process the event, call the step method
/// sm.step();
/// 6. To immediately process the event, call the handle() method
/// sm.handle(MyContext::Event1{});
///
template<typename Context,
         template<class> class Policy = SingleThreadedExecutionPolicy>
using SingleThreadedHsm = Policy<Context>;

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
template<typename Context,
         template<class> class Policy = ThreadedExecutionPolicy>
using ThreadedHsm = Policy<Context>;

///
/// A Periodic state machine. This state machine is driven by a periodic timer.
/// E.g.
/// template<typename Context>
/// using MyPeriodic1HzPolicy =
///     PeriodicExecutionPolicy<Context,
///     ThreadedExecutionPolicy,
///     PeriodicTimer<std::chrono::steady_clock, std::chrono::seconds>>;
/// using MyPeriodic1HzHsm = MyPeriodic1HzPolicy<MyContext>;
/// MyPeriodic1HzHsm sm;
/// sm.start();
/// This will start the state machine and send ClockTick events to the state
/// context every 1 second.
/// To stop the state machine, call sm.stop();
///
template<typename Context,
         template<class> class Policy = PeriodicExecutionPolicy>
using PeriodicHsm = Policy<Context>;

/// Real-time state machine. This state machine is driven by a periodic timer.
/// E.g.
/// template<typename Context>
/// using MyRealtimePeriodic1KhzPolicy =
///    RealtimePeriodicExecutionPolicy<Context,
///     RealtimeExecutionPolicy,
///     PeriodicTimer<std::chrono::steady_clock, std::chrono::milliseconds>>;
/// using MyRealtimePeriodic1KhzHsm = MyRealtimePeriodic1KhzPolicy<MyContext>;
/// MyRealtimePeriodic1KhzHsm sm;
/// sm.start();
///
template<typename Context,
         template<class> class Policy = RealtimePeriodicExecutionPolicy>
using RealtimePeriodicHsm = Policy<Context>;

// Real-time state machine. This state machine is driven by a periodic timer.
template<typename Context,
         template<class> class Policy = RealtimeExecutionPolicy>
using RealtimeHsm = Policy<Context>;

// Concurrent Hsm
template<template<typename> class Policy = ThreadedExecutionPolicy,
         typename... Contexts>
using ConcurrentHsm = make_concurrent_hsm_t<Policy, Contexts...>;

} // namespace tsm
