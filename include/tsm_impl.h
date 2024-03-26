#pragma once
// Lightly tested implementation of a typed HSM in C++20
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#ifdef __FREE_RTOS__
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#else
#include <condition_variable>
#include <mutex>
#include <thread>
#endif // __FREE_RTOS__

#ifdef __linux__
#include <sched.h>
#include <sys/mman.h>
#include <time.h>
#endif
namespace tsm {

// Apply a wrapper to a tuple of types
template<template<class> class Wrapper, typename Tuple>
struct wrap_type_impl;

template<template<class> class Wrapper, typename... Ts>
struct wrap_type_impl<Wrapper, std::tuple<Ts...>> {
    using type = std::tuple<typename Wrapper<Ts>::type...>;
};

template<template<class> class Wrapper, typename Tuple>
using wrap_type = typename wrap_type_impl<Wrapper, Tuple>::type;

// Append unique type to a tuple
template<typename T, typename Tuple>
struct append_unique_impl;

template<typename T>
struct append_unique_impl<T, std::tuple<>> {
    using type = std::tuple<T>;
};

template<typename T, typename... Ts>
struct append_unique_impl<T, std::tuple<Ts...>> {
    // Add T to the tuple if it is not found among Ts..., otherwise, leave as
    // is.
    using type = std::conditional_t<!(std::is_same_v<T, Ts> || ...),
                                    std::tuple<Ts..., T>,
                                    std::tuple<Ts...>>;
};

template<typename T, typename Tuple>
using append_unique = typename append_unique_impl<T, Tuple>::type;

// Remove duplicates from a tuple
// Base case for empty tuple, just provides a type 'type' equal to Us
template<typename Ts, typename Us>
struct unique_tuple_impl;

// Recursive case: tries to append T to Us if it's not already there and
// recurses with rest of Ts
template<typename... Us>
struct unique_tuple_impl<std::tuple<>, std::tuple<Us...>> {
    using type = std::tuple<Us...>;
};

// Recursive case: tries to append T to Us if it's not already there and
// recurses with rest of Ts
template<typename T, typename... Ts, typename... Us>
struct unique_tuple_impl<std::tuple<T, Ts...>, std::tuple<Us...>> {
    using type =
      typename unique_tuple_impl<std::tuple<Ts...>,
                                 append_unique<T, std::tuple<Us...>>>::type;
};

// Public interface, defaults to starting with an empty unique set (Us)
template<typename Ts, typename Us = std::tuple<>>
struct unique_tuple {
    using type = typename unique_tuple_impl<Ts, Us>::type;
};

template<typename Ts>
using unique_tuple_t = typename unique_tuple<Ts>::type;

// Rename tuple to variant
template<typename Tuple>
struct tuple_to_variant_impl;

template<typename... Ts>
struct tuple_to_variant_impl<std::tuple<Ts...>> {
    using type = std::variant<Ts...>;
};

// Pull out the Ts... from a tuple and create a std::
template<typename Tuple>
using tuple_to_variant_t = typename tuple_to_variant_impl<Tuple>::type;

// Event
struct Event {};

template<typename T>
struct wrapped_event : Event {
    using type = T;
};

// Transition
// Dummy action and guard - hopefully, these will be optimized away

template<typename From, typename Event, typename To>
struct BaseTransition {
    using from = From;
    using event = Event;
    using to = To;
};

template<typename From,
         typename Event,
         typename To,
         typename Action = void (*)(),
         typename Guard = bool (*)()>
struct Transition : BaseTransition<From, Event, To> {
    using action = Action;
    using guard = Guard;
    Action action_{}; // for lambdas
    Guard guard_{};
};

struct ClockTickEvent {
    int ticks_;
};

template<typename From,
         typename To,
         typename Guard = bool (*)(void),
         typename Action = void (*)(void)>
struct ClockedTransition
  : Transition<From, ClockTickEvent, To, Action, Guard> {};

// get_states from TransitionTable
template<typename Tuple, typename... Ts>
struct get_states_impl;

// Base case: No more transitions to process, just return the accumulated
// states.
template<typename Tuple>
struct get_states_impl<Tuple> {
    using type = Tuple;
};

// Recursive case: Process the first Transition, then recurse on the rest.
template<typename Tuple, typename First, typename... Rest>
struct get_states_impl<Tuple, First, Rest...> {
    // Accumulate 'from' state
    using from_states = append_unique<typename First::from, Tuple>;
    // Accumulate 'to' state
    using to_states = append_unique<typename First::to, from_states>;

    // Recurse with the updated tuple and the remaining transitions
    using type = typename get_states_impl<to_states, Rest...>::type;
};

template<typename... Ts>
struct get_states;

template<typename... Ts>
struct get_states<std::tuple<Ts...>> {
    using type = typename get_states_impl<std::tuple<>, Ts...>::type;
};

template<typename... Ts>
using get_states_t = typename get_states<Ts...>::type;

// TransitionMap

// Forward declaration for transition_map_helper with a default index parameter
// starting from 0
template<typename From,
         typename Event,
         typename TransitionsTuple,
         size_t Index = 0,
         typename = void>
struct transition_map_helper;

// Helper trait to check if a transition matches 'From' and 'Event'
template<typename Transition, typename From, typename Event>
struct is_transition_match : std::false_type {};

template<typename From,
         typename Event,
         typename To,
         typename Action,
         typename Guard>
struct is_transition_match<Transition<From, Event, To, Action, Guard>,
                           From,
                           Event> : std::true_type {};

// Iteration through the transitions
template<typename From, typename Event, typename... Transitions, size_t Index>
struct transition_map_helper<
  From,
  Event,
  std::tuple<Transitions...>,
  Index,
  std::enable_if_t<(Index < sizeof...(Transitions))>> {
    using CurrentTransition =
      std::tuple_element_t<Index, std::tuple<Transitions...>>;
    static constexpr bool match =
      is_transition_match<CurrentTransition, From, Event>::value;

    using type = typename std::conditional_t<
      match,
      CurrentTransition,
      typename transition_map_helper<From,
                                     Event,
                                     std::tuple<Transitions...>,
                                     Index + 1>::type>;
};

// Base case: Reached the end of the tuple without finding a match
template<typename From, typename Event, typename... Transitions, size_t Index>
struct transition_map_helper<
  From,
  Event,
  std::tuple<Transitions...>,
  Index,
  std::enable_if_t<!(Index < sizeof...(Transitions))>> {
    using type = void; // Return void if no transition matches
};

// Wrapper to start the search
template<typename From, typename Event, typename Transitions>
struct TransitionMap {
    using type = typename transition_map_helper<From, Event, Transitions>::type;
};

// find transition
template<typename From, typename Event, typename Transitions>
using find_transition_t =
  typename TransitionMap<From, Event, Transitions>::type;

// variable template to check for the existence of a valid transition
template<typename State, typename Event, typename Transitions>
inline constexpr bool has_valid_transition_v =
  !std::is_same_v<typename TransitionMap<State, Event, Transitions>::type,
                  void>;

// SFINAE test for exit method
template<typename T, typename Event, typename = void>
struct has_exit : std::false_type {};

template<typename T, typename Event>
struct has_exit<T,
                Event,
                std::void_t<std::disjunction<
                  std::is_invocable<decltype(&T::exit), T>,
                  std::is_invocable<decltype(&T::exit), T, T&>,
                  std::is_invocable<decltype(&T::exit), T, Event>>>>
  : std::true_type {};

template<typename T, typename Event>
inline constexpr bool has_exit_v = has_exit<T, Event>::value;

// SFINAE test for entry method
template<typename T, typename Event, typename = void>
struct has_entry : std::false_type {};

template<typename T, typename Event>
struct has_entry<T,
                 Event,
                 std::void_t<std::disjunction<
                   std::is_invocable<decltype(&T::entry), T>,
                   std::is_invocable<decltype(&T::entry), T&>,
                   std::is_invocable<decltype(&T::entry), T, Event>>>>
  : std::true_type {};

template<typename T, typename Event>
inline constexpr bool has_entry_v = has_entry<T, Event>::value;

// SFINAE test for guard method
template<typename T, typename Event, typename = void>
struct has_guard : std::false_type {};

template<typename T, typename Event>
struct has_guard<T,
                 Event,
                 std::void_t<std::disjunction<
                   std::is_invocable<decltype(&T::guard), T>,
                   std::is_invocable<decltype(&T::guard), T&>,
                   std::is_invocable<decltype(&T::guard), T, Event>>>>
  : std::true_type {};

template<typename T, typename Event>
inline constexpr bool has_guard_v = has_guard<T, Event>::value;

// SFINAE test for action method
template<typename T, typename Event, typename = void>
struct has_action : std::false_type {};

template<typename T, typename Event>
struct has_action<T,
                  Event,
                  std::void_t<std::disjunction<
                    std::is_invocable<decltype(&T::action), T>,
                    std::is_invocable<decltype(&T::action), T&>,
                    std::is_invocable<decltype(&T::action), T, Event>>>>
  : std::true_type {};

template<typename T, typename Event>
inline constexpr bool has_action_v = has_action<T, Event>::value;

// Trait to check for the presence of T::transitions
template<typename, typename = std::void_t<>>
struct has_transitions : std::false_type {};

template<typename T>
struct has_transitions<T, std::void_t<typename T::transitions>>
  : std::true_type {};

template<typename T>
using has_transitions_t = typename has_transitions<T>::type;

template<typename T>
inline constexpr bool has_transitions_v = has_transitions<T>::value;

// Define a trait to check for the presence of State::handle method for a given
// Event and Context
template<typename State,
         typename Event,
         typename Context,
         typename = std::void_t<>>
struct has_handle_method : std::false_type {};

template<typename State, typename Event, typename Context>
struct has_handle_method<
  State,
  Event,
  Context,
  std::void_t<decltype(std::declval<State>().handle(std::declval<Context&>(),
                                                    std::declval<Event>()))>>
  : std::true_type {};

// Helper variable template
template<typename State, typename Event, typename Context>
inline constexpr bool has_handle_method_v =
  has_handle_method<State, Event, Context>::value;

// Trait to check for the presence of T::is_hsm
template<typename, typename = std::void_t<>>
struct is_hsm_trait : std::false_type {};

template<typename T>
struct is_hsm_trait<T, std::void_t<decltype(T::is_hsm)>> : std::true_type {};

template<typename T>
using is_hsm_trait_t = typename is_hsm_trait<T>::type;

template<typename T>
inline constexpr bool is_hsm_trait_v = is_hsm_trait<T>::value;

// Define a helper to check for 'from' and 'to' types in transitions,
template<typename T>
inline constexpr bool is_state_trait_v =
  std::conjunction_v<has_transitions<T>, std::negation<is_hsm_trait<T>>>;

template<typename T, typename = void>
struct is_clocked_hsm : std::false_type {};

template<typename T>
struct is_clocked_hsm<T, std::void_t<decltype(T::is_clocked_hsm)>>
  : std::true_type {};

template<typename T>
inline constexpr bool is_clocked_hsm_v = is_clocked_hsm<T>::value;

// Hsm
template<typename T, typename transitions = typename T::transitions>
struct Hsm : T {
    static constexpr bool is_hsm = true;
    using type = Hsm<T, transitions>;
    using HsmType = type; // alias for policy classes
    using initial_state = typename std::tuple_element_t<0, transitions>::from;
    using States = get_states_t<transitions>;

    Hsm()
      : current_state_(&std::get<initial_state>(states_)) {}

    // for rvalue reference and copy
    template<typename Event>
    bool handle(Event&& e) {
        // using Event = std::decay_t<Evt>;
        // Event e = std::forward<Evt>(event);
        return std::visit(
          [this, &e](auto* state) {
              using State = std::decay_t<decltype(*state)>;
              // check Event is same as decayed e
              bool handled = false;
              // if current_state is a state machine, call handle on it
              if constexpr (is_hsm_trait_t<State>::value) {
                  handled = state->handle(std::forward<Event>(e));
              }
              if (!handled) {
                  // Does State implement handle for Event?
                  if constexpr (has_valid_transition_v<State,
                                                       std::decay_t<Event>,
                                                       transitions>) {
                      using transition = find_transition_t<State,
                                                           std::decay_t<Event>,
                                                           transitions>;
                      this->handle_transition<transition>(
                        state, static_cast<Event&&>(e));
                      handled = true;
                  }
              }
              return handled;
          },
          current_state_);
    }

    template<typename Event, typename State>
    void entry(Event&& e, State* state) noexcept {
        if constexpr (has_entry_v<State, Event>) {
            if constexpr (std::is_invocable_v<decltype(&State::entry),
                                              State*,
                                              T&,
                                              decltype(e)>) {
                state->entry(static_cast<T&>(*this), std::forward<Event>(e));
            } else if constexpr (std::is_invocable_v<decltype(&State::entry),
                                                     State*,
                                                     T&>) {
                state->entry(static_cast<T&>(*this));
            } else if constexpr (std::is_invocable_v<decltype(&State::entry),
                                                     State*>) {
                state->entry();
            }
        }
    }

    template<typename Event, typename State>
    void exit(Event&& e, State* state) noexcept {
        if constexpr (has_exit_v<State, Event>) {
            if constexpr (std::is_invocable_v<decltype(&State::exit),
                                              State*,
                                              T&,
                                              decltype(e)>) {
                state->exit(static_cast<T&>(*this), std::forward<Event>(e));
            } else if constexpr (std::is_invocable_v<decltype(&State::exit),
                                                     State*,
                                                     T&>) {
                state->exit(static_cast<T&>(*this));
            } else if constexpr (std::is_invocable_v<decltype(&State::exit),
                                                     State*>) {
                state->exit();
            }
        }
    }
    // Check Guard
    template<typename Tn,
             typename Event = typename Tn::event,
             typename State = typename Tn::from>
    bool check_guard(Event&& e, State* state) {
        if constexpr (has_guard_v<State, Event>) {
            if constexpr (std::is_invocable_v<decltype(&State::guard),
                                              State*,
                                              T&,
                                              decltype(e)>) {
                return state->guard(static_cast<T&>(*this),
                                    std::forward<Event>(e));
            } else if constexpr (std::is_invocable_v<decltype(&State::guard),
                                                     State*,
                                                     T&>) {
                return state->guard(static_cast<T&>(*this));
            } else if constexpr (std::is_invocable_v<decltype(&State::guard),
                                                     State*>) {
                return state->guard();
            }
        }
        return true;
    }

    // Perform action
    template<typename Tn,
             typename Event = typename Tn::event,
             typename State = typename Tn::from>
    void perform_action(Event&& e, State* state) {
        if constexpr (has_action_v<State, Event>) {
            if constexpr (std::is_invocable_v<decltype(&State::action),
                                              State*,
                                              T&,
                                              decltype(e)>) {
                state->action(static_cast<T&>(*this), std::forward<Event>(e));
            } else if constexpr (std::is_invocable_v<decltype(&State::action),
                                                     State*,
                                                     T&>) {
                state->action(static_cast<T&>(*this));
            } else if constexpr (std::is_invocable_v<decltype(&State::action),
                                                     State*>) {
                state->action();
            }
        }
    }

    template<typename transition,
             typename State = typename transition::from,
             typename Event = typename transition::event>
    std::enable_if_t<!has_handle_method_v<State, Event, T>, void>
    handle_transition(typename transition::from* state, Event&& e) {
        // Assume TransitionMap provides the matching transition
        if (!this->check_guard<transition>(std::forward<Event>(e), state)) {
            return;
        }

        this->template exit<Event, State>(std::forward<Event>(e), state);

        // Optional Action
        this->perform_action<transition>(std::forward<Event>(e), state);

        using to = typename transition::to;

        // switch to the new state
        current_state_ = &std::get<to>(states_);

        this->template entry<Event, to>(std::forward<Event>(e),
                                        *std::get_if<to*>(&current_state_));
    }

    template<typename transition,
             typename State = typename transition::from,
             typename Event = typename transition::event>
    std::enable_if_t<has_handle_method_v<State, Event, T>, void>
    handle_transition(State* state, Event&& e) {

        // A true gives permission to transition
        if (!state->handle(static_cast<T&>(*this), std::forward<Event>(e))) {
            return;
        }

        using to = typename transition::to;

        // switch to the new state
        current_state_ = &std::get<to>(states_);

        this->template entry<Event, to>(std::forward<Event>(e),
                                        *std::get_if<to*>(&current_state_));
    }

    template<typename State>
    void current_state() {
        current_state_ = &std::get<State>(states_);
    }

    States states_;
    transitions transitions_;
    tuple_to_variant_t<wrap_type<std::add_pointer, States>> current_state_;
};

template<typename T, typename = void>
struct make_hsm;

template<typename T>
struct make_hsm<T, std::enable_if_t<!is_state_trait_v<T>>> {
    using type = T;
};

template<typename T>
using make_hsm_t = typename make_hsm<T>::type;

// Recursively wrap states in HSMs if they are state traits
template<typename T>
struct wrap_transition {
    using from = typename T::from;
    using event = typename T::event;
    using to = typename T::to;
    using action = typename T::action;
    using guard = typename T::guard;

    using wrap_from =
      std::conditional_t<is_state_trait_v<from>, make_hsm_t<from>, from>;
    using wrap_to =
      std::conditional_t<is_state_trait_v<to>, make_hsm_t<to>, to>;

    using type = Transition<wrap_from, event, wrap_to, action, guard>;
};

template<typename T>
using wrap_transition_t = typename wrap_transition<T>::type;

template<typename... Ts>
struct wrap_transitions;

template<typename... Ts>
struct wrap_transitions<std::tuple<Ts...>> {
    using type = std::tuple<wrap_transition_t<Ts>...>;
};

template<typename... Ts>
using wrap_transitions_t = typename wrap_transitions<Ts...>::type;

// Clocked HSM - react to events after a certain time period
// This is a wrapper around HSM that adds a tick method to the HSM
template<typename T, template<typename> class Policy = make_hsm_t>
struct ClockedHsm : Policy<T> {
    using type = ClockedHsm<T, Policy>;
    using HsmType = typename Policy<T>::type;

    constexpr static bool is_clocked_hsm = true;

    bool tick() { return this->handle(tick_event_); }

    template<typename Event>
    bool handle(Event e = Event()) {
        return HsmType::handle(e);
    }

    bool handle(ClockTickEvent& e) {
        ++tick_event_.ticks_;
        return HsmType::handle(e);
    }
    ClockTickEvent tick_event_{};
};

// Define the wrapper for state traits to convert them into HSMs including their
// transitions
template<typename T>
struct make_hsm<T, std::enable_if_t<is_state_trait_v<T>>> {
    // shadow the transitions with the wrapped transitions
    using transitions = wrap_transitions_t<typename T::transitions>;

    using type = Hsm<T, transitions>;
};

// Orthogonal HSM
template<typename... Hsms>
struct OrthogonalExecutionPolicy {
    static constexpr bool is_hsm = true;
    using type = OrthogonalExecutionPolicy<Hsms...>;

    template<typename Event>
    void entry(Event e = Event()) {
        std::apply([e](auto&... hsm) { (hsm.entry(e), ...); }, hsms_);
    }

    template<typename Event>
    void exit(Event e = Event()) {
        std::apply([e](auto&... hsm) { (hsm.exit(e), ...); }, hsms_);
    }

    template<typename Event>
    bool handle(Event e = Event()) {
        return std::apply([e](auto&... hsm) { return (hsm.handle(e) && ...); },
                          hsms_);
    }

    std::tuple<Hsms...> hsms_;
};

// A thread safe event queue. Any thread can call add_event if it has a pointer
// to the event queue. The call to nextEvent is a blocking call
template<typename Event, typename LockType, typename ConditionVarType>
struct EventQueueT {
    using EventType = Event;

    virtual ~EventQueueT() { stop(); }

  public:
    // Block until you get an event
    Event next_event() {
        std::unique_lock<LockType> lock(eventQueueMutex_);
        cvEventAvailable_.wait(
          lock, [this] { return (!this->empty() || this->interrupt_); });
        if (interrupt_) {
            return Event();
        }
        const Event e = std::move(front());
        pop_front();
        return e;
    }

    void add_event(Event&& e) {
        if (interrupt_) {
            return;
        }
        std::lock_guard<LockType> lock(eventQueueMutex_);
        push_back(std::forward<Event>(e));
        cvEventAvailable_.notify_all();
    }

    void stop() {
        interrupt_ = true;
        cvEventAvailable_.notify_all();
        // Log the events that are going to get dumped if the queue is not
        // empty
    }

    bool interrupted() { return interrupt_; }

  protected:
    bool empty() { return push_index_ == pop_index_; }

    Event const& front() { return data_[pop_index_]; }

    void pop_front() {
        if (!empty()) {
            pop_index_ = (pop_index_ + 1) % data_.size();
        }
    }

    bool push_back(Event const& e) {
        if ((push_index_ + 1) % data_.size() != pop_index_) {
            data_[push_index_] = e;
            push_index_ = (push_index_ + 1) % data_.size();
            return true;
        }
        return false;
    }

  private:
    LockType eventQueueMutex_;
    ConditionVarType cvEventAvailable_;
    bool interrupt_{};
    size_t push_index_{ 0 };
    size_t pop_index_{ 0 };
    std::array<Event, 50> data_;
};

#ifdef __FREE_RTOS__
constexpr int MaxEvents = 10; // Define your own queue size

class FreeRTOSMutex {
  public:
    FreeRTOSMutex() {
        // Create the FreeRTOS mutex
        this->mutex = xSemaphoreCreateMutex();
    }

    ~FreeRTOSMutex() {
        // Delete the FreeRTOS mutex
        vSemaphoreDelete(this->mutex);
    }

    void lock() {
        // Wait forever for the mutex to become available
        xSemaphoreTake(this->mutex, portMAX_DELAY);
    }

    bool try_lock() {
        // Try to take the mutex without blocking
        return xSemaphoreTake(this->mutex, 0) == pdTRUE;
    }

    void unlock() {
        // Release the mutex
        xSemaphoreGive(this->mutex);
    }

    // Return the native handle of the mutex
    SemaphoreHandle_t native_handle() { return this->mutex; }

  private:
    SemaphoreHandle_t mutex;
};

class FreeRTOSConditionVariable {
  public:
    FreeRTOSConditionVariable()
      : semaphore_(xSemaphoreCreateBinary())
      , waitersCount_(0) {
        // Ensure the semaphore is in a non-signalled state initially.
        xSemaphoreTake(semaphore_, 0);
    }

    ~FreeRTOSConditionVariable() { vSemaphoreDelete(semaphore_); }

    template<typename Lock>
    void wait(Lock& lock) {
        // Increment the count of waiters.
        UBaseType_t oldISRState = taskENTER_CRITICAL_FROM_ISR();
        ++waitersCount_;
        taskEXIT_CRITICAL_FROM_ISR(oldISRState);

        // Unlock the external mutex and wait for notification.
        lock.unlock();
        xSemaphoreTake(semaphore_, portMAX_DELAY);

        // Decrement the count of waiters.
        oldISRState = taskENTER_CRITICAL_FROM_ISR();
        --waitersCount_;
        taskEXIT_CRITICAL_FROM_ISR(oldISRState);

        // Re-acquire the mutex before exiting.
        lock.lock();
    }

    void notify_one() {
        if (waitersCount_ > 0) {        // Check if there are any waiters.
            xSemaphoreGive(semaphore_); // Wake up one waiter.
        }
    }

    void notify_all() {
        while (waitersCount_ > 0) { // Keep waking all waiters.
            xSemaphoreGive(semaphore_);
            // Small delay to allow other tasks to respond to the semaphore.
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

  private:
    SemaphoreHandle_t semaphore_;
    volatile UBaseType_t waitersCount_;
};

// C++ 11 compatible clock with FreeRTOS tick count.
struct FreeRTOSClock {
    using duration = std::chrono::milliseconds;
    using period = duration::period;
    using rep = duration::rep;
    using time_point = std::chrono::time_point<FreeRTOSClock>;

    static constexpr bool is_steady = true;

    static time_point now() noexcept {
        TickType_t ticks = xTaskGetTickCount();
        // Convert ticks to milliseconds (note: configTICK_RATE_HZ is the number
        // of ticks per second)
        auto durationSinceEpoch =
          std::chrono::milliseconds(ticks * (1000 / configTICK_RATE_HZ));
        return time_point(duration(durationSinceEpoch));
    }
};

template<typename Event>
using EventQueue = EventQueueT<Event, FreeRTOSMutex, FreeRTOSConditionVariable>;

template<typename HsmType, typename Events>
class ThreadedExecutionPolicy : public HsmType {
  public:
    using Event = tuple_to_variant_t<Events>;
    // using EventQueue = FreeRTOSEventQueue<Event>; // Adapted for FreeRTOS
    using TaskCallback = void (*)(ThreadedExecutionPolicy*);

    ThreadedExecutionPolicy()
      : taskCallback(ThreadedExecutionPolicy::StepTask) {
        interrupt_ = pdFALSE;
        xTaskCreate(reinterpret_cast<TaskFunction_t>(taskCallback),
                    "ThreadedPolicyTask",
                    configMINIMAL_STACK_SIZE,
                    this,
                    tskIDLE_PRIORITY,
                    &smTaskHandle);
    }

    virtual ~ThreadedExecutionPolicy() {
        interrupt_ = pdTRUE;
        // Proper FreeRTOS task deletion if needed, ensuring clean-up.
        if (smTaskHandle != nullptr) {
            vTaskDelete(smTaskHandle);
            smTaskHandle = nullptr;
        }
    }

    void send_event(const Event& event) { eventQueue.add_event(event); }

  protected:
    TaskCallback taskCallback{};
    TaskHandle_t smTaskHandle{};
    EventQueue eventQueue;
    BaseType_t interrupt_;

    static void StepTask(void* pvParameters) {
        auto* policy = static_cast<ThreadedExecutionPolicy*>(pvParameters);
        policy->step();
    }

    void step() {
        while (!interrupt_) {
            processEvent();
        }
    }

    void process_event() {
        Event const& nextEvent = eventQueue.next_event();
        if (!eventQueue.interrupted()) {
            std::visit([this](auto const& e) { return this->handle(e); },
                       nextEvent);
        }
    }
};

#else // __FREE_RTOS__ is not defined

template<typename Event>
using EventQueue = EventQueueT<Event, std::mutex, std::condition_variable_any>;

// C++ 11 compatible accurate clock (nanosecond precision). This is a drop-in
// replacement for Clock types in std::chrono
struct AccurateClock {
    using duration = std::chrono::nanoseconds;
    using period = duration::period;
    using rep = duration::rep;
    using time_point = std::chrono::time_point<AccurateClock>;

    static constexpr bool is_steady = true;

    static time_point now() noexcept {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        auto durationSinceEpoch = std::chrono::seconds(ts.tv_sec) +
                                  std::chrono::nanoseconds(ts.tv_nsec);
        return time_point(duration(durationSinceEpoch));
    }
};

#endif

// Forward declarations
template<typename HsmType, typename = void>
struct get_events_from_hsm;

template<typename Transition>
struct get_events_from_transition;

template<typename TransitionsTuple>
struct aggregate_events;

// Specialize for transitions tuple
template<typename... Transitions>
struct aggregate_events<std::tuple<Transitions...>> {
    using type = decltype(std::tuple_cat(
      typename get_events_from_transition<Transitions>::type{}...));
};

// Base case for empty tuple
template<>
struct aggregate_events<std::tuple<>> {
    using type = std::tuple<>;
};

// Extract events from a single transition
template<typename Transition>
struct get_events_from_transition {
    using Event = std::tuple<typename Transition::event>;
    using FromEvents =
      typename get_events_from_hsm<typename Transition::from>::type;
    using ToEvents =
      typename get_events_from_hsm<typename Transition::to>::type;
    using type = decltype(std::tuple_cat(Event{}, FromEvents{}, ToEvents{}));
};

// Main structure for extracting events from an HSM
template<typename HsmType>
struct get_events_from_hsm<HsmType,
                           std::enable_if_t<has_transitions_v<HsmType>>> {
    using Transitions = typename HsmType::transitions;
    using type = typename aggregate_events<Transitions>::type;
};

// Fallback for non-HSM types
template<typename HsmType>
struct get_events_from_hsm<HsmType,
                           std::enable_if_t<!has_transitions_v<HsmType>>> {
    using type = std::tuple<>;
};

// Helper alias
template<typename HsmType>
using get_events_t =
  unique_tuple_t<typename get_events_from_hsm<HsmType>::type>;

// Single threaded execution policy
template<typename Context, template<typename> class Policy = make_hsm_t>
struct SingleThreadedExecutionPolicy : Policy<Context> {
    using type = SingleThreadedExecutionPolicy<Context, Policy>;
    using HsmType = typename Policy<Context>::type;
    using Event = tuple_to_variant_t<get_events_t<HsmType>>;

    bool step() {
        // This is a blocking wait
        Event const& nextEvent = eventQueue_.next_event();
        // go down the Hsm hierarchy to handle the event as that is the
        // "most active state"
        return std::visit(
          [this](auto&& e) -> bool { return HsmType::handle(e); }, nextEvent);
    }

    void send_event(Event&& event) {
        eventQueue_.add_event(std::forward<Event>(event));
    }

  private:
    EventQueue<Event> eventQueue_;
    bool interrupt_{};
};

// Asynchronous execution policy
template<typename Context, template<typename> class Policy = make_hsm_t>
struct ThreadedExecutionPolicy : Policy<Context> {
    using type = ThreadedExecutionPolicy<Context, Policy>;
    using HsmType = typename Policy<Context>::type;
    using Event = tuple_to_variant_t<get_events_t<HsmType>>;

    void start() {
        smThread_ = std::thread([this] {
            while (!interrupt_) {
                this->process_event();
            }
        });
    }

    void stop() {
        eventQueue_.stop();
        interrupt_ = true;
        if (smThread_.joinable()) {
            smThread_.join();
        }
    }

    virtual ~ThreadedExecutionPolicy() { stop(); }

    void send_event(Event&& event) {
        eventQueue_.add_event(std::forward<Event>(event));
    }

  protected:
    std::thread smThread_;
    EventQueue<Event> eventQueue_;
    bool interrupt_{};

    void process_event() {
        // This is a blocking wait
        Event const& nextEvent = eventQueue_.next_event();
        if (!eventQueue_.interrupted()) {
            std::visit([this](auto const& e) { return this->handle(e); },
                       nextEvent);
        }
    }
};

///
/// A simple observer class. The notify method will be invoked by an
/// AsyncExecWithObserver state machine after event processing. This observer
/// also implements a blocking wait so that test methods can invoke the wait
/// method to synchronize with the event processing.
///
template<typename LockType, typename ConditionVarType>
struct BlockingObserverT {
    void notify() {
        std::unique_lock<std::mutex> lock(smBusyMutex_);
        notified_ = true;
        cv_.notify_all();
    }

    void wait() {
        std::unique_lock<std::mutex> lock(smBusyMutex_);
        cv_.wait(lock, [this] { return this->notified_ == true; });
        notified_ = false;
    }

  private:
    LockType smBusyMutex_;
    ConditionVarType cv_;
    bool notified_{};
};

// Preserving for historical reasons
struct CallbackObserver {
    void add_callback(std::function<void()>&& cb) {
        if (cb != nullptr) {
            cbs_.push_back(cb);
        }
    }

    void notify() {
        for (auto const& cb : cbs_) {
            cb();
        }
    }

  private:
    std::vector<std::function<void()>> cbs_;
};

#ifdef __FREE_RTOS__
using BlockingObserver =
  BlockingObserverT<FreeRTOSMutex, FreeRTOSConditionVariable>;
#else
using BlockingObserver = BlockingObserverT<std::mutex, std::condition_variable>;
#endif

///
/// Another Threaded execution policy. The only difference with
/// ThreadedExecutionPolicy is that an Observer's notify method will be invoked
/// at the end of processing each event - specifically, right before the
/// blocking wait for the next event.
///
template<typename Observer,
         typename Context,
         template<typename> class Policy = ThreadedExecutionPolicy>
struct ThreadedExecWithObserver
  : public Policy<Context>
  , public Observer {
    using type = ThreadedExecWithObserver<Observer, Context, Policy>;
    using HsmType = typename Policy<Context>::type;
    using HsmType::interrupt_;
    using HsmType::process_event;
    using HsmType::smThread_;

    void start() {
        smThread_ = std::thread([this] {
            while (!interrupt_) {
                Observer::notify();
                process_event();
            }
        });
    }
    virtual ~ThreadedExecWithObserver() = default;
};

///
/// Execution policy that uses a BlockingObserver.
/// This is illustrative of chaining policies together.
///
template<typename Context>
using ThreadedBlockingObserver =
  ThreadedExecWithObserver<Context, BlockingObserver>;

/// Helper to implement a Timed Execution Policy
template<typename Clock = std::chrono::steady_clock,
         typename Duration = typename Clock::duration>
struct Timer {
    using ClockType = Clock;
    using DurationType = Duration;
    void start() {
        start_time_ = Clock::now();
        started_ = true;
    }

    Duration elapsed() const {
        if (!started_) {
            return Duration(0);
        }
        auto now = Clock::now();
        auto interval = now - start_time_;
        return std::chrono::duration_cast<Duration>(interval);
    }

    template<typename ToDuration = Duration>
    Duration elapsed(ToDuration since) const {
        auto now = Clock::now();
        if constexpr (std::is_same<ToDuration, Duration>::value) {
            return std::chrono::duration_cast<Duration>(now - since);
        } else {
            return std::chrono::duration_cast<ToDuration>(now - since);
        }
    }
    // template function to convert to a different Duration
    template<typename ToDuration>
    ToDuration elapsed() const {
        return std::chrono::duration_cast<ToDuration>(elapsed());
    }

    bool started() const { return started_; }
    void reset() { start_time_ = Clock::now(); }
    void stop() { started_ = false; }

  protected:
    typename Clock::time_point start_time_;
    bool started_{ false };
};

// useful for profiling
template<typename Clock = std::chrono::steady_clock,
         typename Duration = typename Clock::duration>
struct IntervalTimer : public Timer<Clock, Duration> {
    // Return the interval since the previous call to this function, typecast
    // to the Duration type
    Duration interval() {
        if (!this->started()) {
            this->start();
            return Duration(0);
        }
        auto now = Clock::now();
        auto interval = now - this->start_time_;
        this->start_time_ = now;
        return std::chrono::duration_cast<Duration>(interval);
    }
};

#ifdef __linux__
// Periodic Timer
// Lock up the calling thread for a period of time. Accuracy is determined by
// real-time scheduler settings and OS scheduling policies
template<typename Clock = AccurateClock,
         typename Duration = typename Clock::duration>
struct PeriodicSleepTimer : public Timer<Clock, Duration> {
    PeriodicSleepTimer(Duration period = Duration(1))
      : period_(period) {}
    void start() { Timer<Clock, Duration>::start(); }

    void wait() {
        // calculate time elapsed since last callback
        auto remaining = period_ - Timer<Clock, Duration>::elapsed();
        // nanosleep for remaining time
        // Convert duration to timespec
        struct timespec ts;
        ts.tv_sec =
          std::chrono::duration_cast<std::chrono::seconds>(remaining).count();
        ts.tv_nsec =
          std::chrono::duration_cast<std::chrono::nanoseconds>(remaining)
            .count();
        // remaining time if -1
        struct timespec remaining_ts;
        while (nanosleep(&ts, &remaining_ts) == EINTR) {
            ts = remaining_ts;
        }
        // ensure that callback finishes within the period
        Timer<Clock, Duration>::reset();
    }

    Duration get_period() const { return period_; }

  protected:
    Duration period_;
};

// Real-time execution policy
struct RealtimeConfigurator {
    RealtimeConfigurator() = default;
    RealtimeConfigurator(int priority, std::array<int, 4> affinity)
      : PROCESS_PRIORITY(priority)
      , CPU_AFFINITY(affinity) {}

    void config_realtime_thread() {
        // Set thread to real-time priority
        struct sched_param param;
        param.sched_priority = PROCESS_PRIORITY - 3;
        if (pthread_setschedparam(pthread_self(), SCHED_RR, &param) != 0) {
            perror("pthread_setschedparam");
        }

        // Set CPU affinity
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        for (auto cpu : CPU_AFFINITY) {
            CPU_SET(cpu, &cpuset);
        }

        if (pthread_setaffinity_np(
              pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
            perror("sched_setaffinity");
        }

        // Lock memory
        if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
            perror("mlockall failed");
        }
    }

    template<typename Fn>
    std::thread real_time_thread(Fn fn) {
        return std::thread([this, fn] {
            this->config_realtime_thread();
            fn();
        });
    }

    std::thread make_real_time(std::thread&& t) {
        config_realtime_thread();
        return std::move(t);
    }

  protected:
    int PROCESS_PRIORITY{ 98 };
    std::array<int, 4> CPU_AFFINITY{ 0, 1, 2, 3 };
};

// Real-time execution policy - set cpu isolation in grub
template<typename Context,
         template<typename> class Policy = ThreadedExecutionPolicy>
struct RealtimeExecutionPolicy
  : Policy<Context>
  , RealtimeConfigurator {
    using type = RealtimeExecutionPolicy<Context, Policy>;
    using HsmType = typename Policy<Context>::type;
    using HsmType::interrupt_;
    using HsmType::process_event;
    using HsmType::smThread_;

    void start() {
        smThread_ = RealtimeConfigurator::real_time_thread([this] {
            while (!interrupt_) {
                process_event();
            }
        });
    }

    virtual ~RealtimeExecutionPolicy() = default;
};

// Periodic execution policy
template<typename Context,
         template<typename> class Policy = ThreadedExecutionPolicy,
         typename PeriodicTimer = PeriodicSleepTimer<std::chrono::steady_clock,
                                                     std::chrono::milliseconds>>
struct PeriodicExecutionPolicy
  : Policy<Context>
  , PeriodicTimer {
    using type = PeriodicExecutionPolicy<Context, Policy, PeriodicTimer>;
    using HsmType = typename Policy<Context>::type;
    using TimerType = PeriodicTimer;
    using HsmType::interrupt_;
    using HsmType::send_event;

    void start() {
        PeriodicTimer::start();
        ThreadedExecutionPolicy<Context>::start();

        eventThread_ = std::thread([this] {
            while (!interrupt_) {
                PeriodicTimer::wait();
                ++tick_event_.ticks_;
                this->send_event(tick_event_);
            }
        });
    }

    void stop() {
        ThreadedExecutionPolicy<Context>::stop();
        if (eventThread_.joinable()) {
            eventThread_.join();
        }
    }
    virtual ~PeriodicExecutionPolicy() { stop(); }

    int get_ticks() { return tick_event_.ticks_; }

  protected:
    std::thread eventThread_;
    ClockTickEvent tick_event_;
};

// Periodic Real-time execution policy
template<typename Context,
         template<typename> class Policy = ThreadedExecutionPolicy,
         typename PeriodicTimer = PeriodicSleepTimer<std::chrono::steady_clock,
                                                     std::chrono::milliseconds>>
struct RealtimePeriodicExecutionPolicy
  : RealtimeConfigurator
  , Policy<Context>
  , PeriodicTimer {
    using type =
      RealtimePeriodicExecutionPolicy<Context, Policy, PeriodicTimer>;
    using TimerType = PeriodicTimer;
    using HsmType = typename Policy<Context>::type;
    using HsmType::interrupt_;
    using HsmType::process_event;
    using HsmType::send_event;
    using HsmType::smThread_;

    void start() {
        PeriodicTimer::start();
        smThread_ = RealtimeConfigurator::real_time_thread([this] {
            while (!interrupt_) {
                this->process_event();
            }
        });

        eventThread_ = RealtimeConfigurator::real_time_thread([this] {
            while (!interrupt_) {
                PeriodicTimer::wait();
                ++tick_event_.ticks_;
                this->send_event(tick_event_);
            }
        });
    }

    void stop() {
        ThreadedExecutionPolicy<Context>::stop();
        if (eventThread_.joinable()) {
            eventThread_.join();
        }
    }
    virtual ~RealtimePeriodicExecutionPolicy() { stop(); }

    int get_ticks() { return tick_event_.ticks_; }

  protected:
    std::thread eventThread_;
    ClockTickEvent tick_event_;
};

// Concurrent HSMs
template<typename... Hsms>
struct ConcurrentExecutionPolicy {
    static constexpr bool is_hsm = true;
    using type = ConcurrentExecutionPolicy<Hsms...>;

    template<typename Event>
    void entry(Event e = Event()) {
        std::apply([e](auto&... hsm) { (hsm.entry(e), ...); }, hsms_);
    }

    template<typename Event>
    void exit(Event&& e) {
        std::apply([e](auto&... hsm) { (hsm.exit(e), ...); }, hsms_);
    }

    template<typename Event>
    void send_event(Event&& e) {
        return std::apply(
          // send event to all HSMs
          [e](auto&... hsm) { (hsm.send_event(e), ...); },
          hsms_);
    }

    // assume hsms can be `tick`ed

    void tick() {
        std::apply([](auto&... hsm) { (hsm.tick(), ...); }, hsms_);
    }

    std::tuple<Hsms...> hsms_;
};

// Each HSM in the concurrent HSM is wrapped with a ThreadedExecutionPolicy
template<template<typename> class Policy = ThreadedExecutionPolicy,
         typename... Ts>
struct make_concurrent_hsm {
    using type = ConcurrentExecutionPolicy<Policy<Ts>...>;
};

template<template<typename> class Policy = ThreadedExecutionPolicy,
         typename... Ts>
using make_concurrent_hsm_t = typename make_concurrent_hsm<Policy, Ts...>::type;

#endif // __linux__

} // namespace hsm
