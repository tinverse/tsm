#pragma once
// Lightly tested implementation of a typed HSM in C++20
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include <iostream>

// Apply a wrapper to a tuple of types
template<template<class> class Wrapper, typename Tuple>
struct wrap_type_impl;

template<template<class> class Wrapper, typename... Ts>
struct wrap_type_impl<Wrapper, std::tuple<Ts...>>
{
    using type = std::tuple<typename Wrapper<Ts>::type...>;
};

template<template<class> class Wrapper, typename Tuple>
using wrap_type = typename wrap_type_impl<Wrapper, Tuple>::type;

// Append unique type to a tuple
template<typename T, typename Tuple>
struct append_unique_impl;

template<typename T>
struct append_unique_impl<T, std::tuple<>>
{
    using type = std::tuple<T>;
};

template<typename T, typename... Ts>
struct append_unique_impl<T, std::tuple<Ts...>>
{
    // Add T to the tuple if it is not found among Ts..., otherwise, leave as
    // is.
    using type = std::conditional_t<!(std::is_same_v<T, Ts> || ...),
                                    std::tuple<Ts..., T>,
                                    std::tuple<Ts...>>;
};

template<typename T, typename Tuple>
using append_unique = typename append_unique_impl<T, Tuple>::type;

// Rename tuple to variant
template<typename Tuple>
struct tuple_to_variant_impl;

template<typename... Ts>
struct tuple_to_variant_impl<std::tuple<Ts...>>
{
    using type = std::variant<Ts...>;
};

// Pull out the Ts... from a tuple and create a std::
template<typename Tuple>
using tuple_to_variant_t = typename tuple_to_variant_impl<Tuple>::type;

// Transition
// Dummy action and guard - hopefully, these will be optimized away
auto dummy_action = [](auto&&) {};
auto dummy_guard = [](auto&&) { return true; };

template<typename From, typename Event, typename To>
struct BaseTransition
{
    using from = From;
    using event = Event;
    using to = To;
};

template<typename From,
         typename Event,
         typename To,
         typename Action = void,
         typename Guard = void>
struct Transition : BaseTransition<From, Event, To>
{
    using action = Action;
    using guard = Guard;
    // These will only be used if Action and Guard are not void.
    std::conditional_t<!std::is_same_v<Action, void>,
                       Action,
                       decltype(dummy_action)>
      action_{};
    std::
      conditional_t<!std::is_same_v<Guard, void>, Guard, decltype(dummy_guard)>
        guard_{};
};

struct ClockTickEvent
{};
template<typename From,
         typename To,
         typename Guard = void,
         typename Action = void>
struct ClockedTransition : Transition<From, ClockTickEvent, To, Action, Guard>
{
};

// get_states from TransitionTable
template<typename Tuple, typename... Ts>
struct get_states_impl;

// Base case: No more transitions to process, just return the accumulated
// states.
template<typename Tuple>
struct get_states_impl<Tuple>
{
    using type = Tuple;
};

// Recursive case: Process the first Transition, then recurse on the rest.
template<typename Tuple, typename First, typename... Rest>
struct get_states_impl<Tuple, First, Rest...>
{
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
struct get_states<std::tuple<Ts...>>
{
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
struct is_transition_match : std::false_type
{
};

template<typename From,
         typename Event,
         typename To,
         typename Action,
         typename Guard>
struct is_transition_match<Transition<From, Event, To, Action, Guard>,
                           From,
                           Event> : std::true_type
{
};

// Iteration through the transitions
template<typename From, typename Event, typename... Transitions, size_t Index>
struct transition_map_helper<From,
                             Event,
                             std::tuple<Transitions...>,
                             Index,
                             std::enable_if_t<(Index < sizeof...(Transitions))>>
{
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
  std::enable_if_t<!(Index < sizeof...(Transitions))>>
{
    using type = void; // Return void if no transition matches
};

// Wrapper to start the search
template<typename From, typename Event, typename Transitions>
struct TransitionMap
{
    using type = typename transition_map_helper<From, Event, Transitions>::type;
};

// variable template to check for the existence of a valid transition
template<typename State, typename Event, typename Transitions>
inline constexpr bool has_valid_transition_v =
  !std::is_same_v<typename TransitionMap<State, Event, Transitions>::type,
                  void>;

// SFINAE test for exit method
template<typename T, typename = void>
struct has_exit : std::false_type
{
};

template<typename T>
struct has_exit<
  T,
  std::void_t<std::disjunction<std::is_invocable<decltype(&T::exit), T>,
                               std::is_invocable<decltype(&T::exit), T, T&>>>>
  : std::true_type
{
};

template<typename T>
inline constexpr bool has_exit_v = has_exit<T>::value;

// SFINAE test for entry method
template<typename T, typename = void>
struct has_entry : std::false_type
{
};

template<typename T>
struct has_entry<
  T,
  std::void_t<std::disjunction<std::is_invocable<decltype(&T::entry), T>,
                               std::is_invocable<decltype(&T::entry), T&>>>>
  : std::true_type
{
};

template<typename T>
inline constexpr bool has_entry_v = has_entry<T>::value;

// Trait to check for the presence of T::transitions
template<typename, typename = std::void_t<>>
struct has_transitions : std::false_type
{
};

template<typename T>
struct has_transitions<T, std::void_t<typename T::transitions>> : std::true_type
{
};

template<typename T>
using has_transitions_t = typename has_transitions<T>::type;

template<typename T>
inline constexpr bool has_transitions_v = has_transitions<T>::value;

// Trait to check for the presence of T::is_hsm
template<typename, typename = std::void_t<>>
struct is_hsm_trait : std::false_type
{
};

template<typename T>
struct is_hsm_trait<T, std::void_t<decltype(T::is_hsm)>> : std::true_type
{
};

template<typename T>
using is_hsm_trait_t = typename is_hsm_trait<T>::type;

template<typename T>
inline constexpr bool is_hsm_trait_v = is_hsm_trait<T>::value;

// Define a helper to check for 'from' and 'to' types in transitions,
template<typename T>
inline constexpr bool is_state_trait_v =
  std::conjunction_v<has_transitions<T>, std::negation<is_hsm_trait<T>>>;

// Check if T has ::base_traits defined
template<typename T, typename = void>
struct has_base_trait : std::false_type
{
};

template<typename T>
struct has_base_trait<T, std::void_t<typename T::base_trait>> : std::true_type
{
};

template<typename T>
inline constexpr bool has_base_trait_v = has_base_trait<T>::value;

struct clocked_trait
{
    constexpr static bool is_clocked = true;
    int ticks_{};
};
// Check if type has is_clocked or inherits from a type with is_clocked
template<typename T, typename = void>
struct is_clocked_trait : std::false_type
{
};

template<typename T>
struct is_clocked_trait<T, std::void_t<decltype(T::is_clocked)>>
  : std::true_type
{
};

template<typename T>
inline constexpr bool is_clocked_trait_v = is_clocked_trait<T>::value;

template<typename T>
struct Hsm : T
{
    static constexpr bool is_hsm = true;
    // set base_traits to T if not already set in T
    using base_trait =
      std::conditional_t<has_base_trait_v<T>, typename T::base_trait, T>;
    using type = Hsm<T>;
    using transitions = typename T::transitions;
    using initial_state = typename std::tuple_element_t<0, transitions>::from;
    using States = get_states_t<transitions>;

    Hsm()
    {
        current_state_ = &std::get<initial_state>(states_);
        if constexpr (has_entry_v<initial_state>) {
            initial_state* state = std::get<initial_state*>(current_state_);
            if constexpr (std::is_invocable_v<decltype(&initial_state::entry),
                                              initial_state*,
                                              base_trait&>) {
                state->entry(static_cast<base_trait&>(*this));
            } else {
                state->entry();
            }
        }
    }

    template<typename Event>
    void entry(Event e = Event())
    {
        std::visit(
          [this, e](auto* state) {
              using State = std::decay_t<decltype(*state)>;
              if constexpr (is_hsm_trait_v<State>) {
                  std::get<State>(states_).entry(e);
              } else {
                  if constexpr (has_entry_v<State>) {
                      State* state = std::get<State*>(current_state_);
                      if constexpr (std::is_invocable_v<decltype(&State::entry),
                                                        State*,
                                                        base_trait&>) {
                          state->entry(static_cast<base_trait&>(*this));
                      } else {
                          state->entry();
                      }
                  }
              }
          },
          current_state_);
    }

    template<typename Event>
    void exit(Event e = Event())
    {
        std::visit(
          [this, e](auto* state) {
              using State = std::decay_t<decltype(*state)>;
              if constexpr (is_hsm_trait_v<State>) {
                  std::get<State>(states_).exit(e);
              } else {
                  if constexpr (has_exit_v<State>) {
                      State* state = std::get<State*>(current_state_);
                      if constexpr (std::is_invocable_v<decltype(&State::exit),
                                                        State*,
                                                        base_trait&>) {
                          state->exit(static_cast<base_trait&>(*this));
                      } else {
                          state->exit();
                      }
                  }
              }
          },
          current_state_);
    }

    template<typename Event>
    bool handle(Event e = Event())
    {
        return std::visit(
          [this, e](auto* state) {
              using State = std::decay_t<decltype(*state)>;
              bool handled = false;
              // if current_state is a state machine, call handle on it
              if constexpr (is_hsm_trait_t<State>::value) {
                  handled = std::get<State>(states_).handle(e);
              }
              if (!handled) {
                  // Does State implement handle for Event?
                  if constexpr (has_valid_transition_v<State,
                                                       Event,
                                                       transitions>) {
                      this->handleEventForState<Event>(state);
                      handled = true;
                  }
              }
              return handled;
          },
          current_state_);
    }

    // Check Guard
    template<typename Tn, typename State = typename Tn::from>
    bool check_guard(State* state)
    {
        if constexpr (std::is_invocable_v<typename Tn::guard, base_trait&>) {
            Tn& t = std::get<Tn>(transitions_);
            return std::invoke(t.guard_, static_cast<base_trait&>(*this));
        } else if constexpr (std::is_invocable_v<typename Tn::guard,
                                                 State*,
                                                 base_trait&>) {

            return state->guard(static_cast<base_trait&>(*this));
        }
        return true;
    }

    // Perform action
    template<typename Tn, typename State = typename Tn::from>
    void perform_action(State* state)
    {
        if constexpr (std::is_invocable_v<typename Tn::action, base_trait&>) {
            Tn& t = std::get<Tn>(transitions_);
            std::invoke(t.action_, static_cast<base_trait&>(*this));
        } else if constexpr (std::is_invocable_v<typename Tn::action,
                                                 State*,
                                                 base_trait&>) {
            state->action(static_cast<base_trait&>(*this));
        }
    }
    template<typename Event, typename State>
    void handleEventForState(State* state)
    {
        // Assume TransitionMap provides the matching transition
        using transition =
          typename TransitionMap<State, Event, transitions>::type;

        if (!this->check_guard<transition>(state)) {
            return;
        }

        this->exit(Event());

        // Optional Action
        this->perform_action<transition>(state);

        using to = typename transition::to;

        // switch to the new state
        current_state_ = &std::get<to>(states_);

        this->entry(Event());
    }

    template<typename State>
    void current_state()
    {
        current_state_ = &std::get<State>(states_);
    }

    States states_;
    transitions transitions_;
    tuple_to_variant_t<wrap_type<std::add_pointer, States>> current_state_;
};

template<typename T, typename = void>
struct convert_to_hsm;

template<typename T>
struct convert_to_hsm<T, std::enable_if_t<!is_state_trait_v<T>>>
{
    using type = T;
};

template<typename T>
using convert_to_hsm_t = typename convert_to_hsm<T>::type;

// Recursively wrap states in HSMs if they are state traits
template<typename T>
struct wrap_transition
{
    using from = typename T::from;
    using event = typename T::event;
    using to = typename T::to;
    using action = typename T::action;
    using guard = typename T::guard;

    using wrap_from =
      std::conditional_t<is_state_trait_v<from>, convert_to_hsm_t<from>, from>;
    using wrap_to =
      std::conditional_t<is_state_trait_v<to>, convert_to_hsm_t<to>, to>;

    using type = Transition<wrap_from, event, wrap_to, action, guard>;
};

template<typename T>
using wrap_transition_t = typename wrap_transition<T>::type;

template<typename... Ts>
struct wrap_transitions;

template<typename... Ts>
struct wrap_transitions<std::tuple<Ts...>>
{
    using type = std::tuple<wrap_transition_t<Ts>...>;
};

template<typename... Ts>
using wrap_transitions_t = typename wrap_transitions<Ts...>::type;

// Clocked HSM - react to events after a certain time period
// This is a wrapper around HSM that adds a tick method to the HSM
template<typename HsmType>
struct ClockedHsm : HsmType
{
    using type = ClockedHsm<HsmType>;
    bool tick()
    {
        this->ticks_++;
        return HsmType::handle(ClockTickEvent());
    }
};

// Define the wrapper for state traits to convert them into HSMs including their
// transitions
template<typename T>
struct convert_to_hsm<T, std::enable_if_t<is_state_trait_v<T>>>
{
    struct Traits : T
    {
        // base traits for callbacks
        using base_trait = T;

        // shadow the transitions with the wrapped transitions
        using transitions = wrap_transitions_t<typename T::transitions>;
    };

    using type = std::conditional_t<is_clocked_trait_v<T>,
                                    ClockedHsm<Hsm<Traits>>,
                                    Hsm<Traits>>;
};

// Orthogonal HSM
template<typename... Hsms>
struct OrthogonalHsm
{
    static constexpr bool is_hsm = true;
    using type = OrthogonalHsm<Hsms...>;

    template<typename Event>
    void entry(Event e = Event())
    {
        std::apply([e](auto&... hsm) { (hsm.entry(e), ...); }, hsms_);
    }

    template<typename Event>
    void exit(Event e = Event())
    {
        std::apply([e](auto&... hsm) { (hsm.exit(e), ...); }, hsms_);
    }

    template<typename Event>
    bool handle(Event e = Event())
    {
        return std::apply([e](auto&... hsm) { return (hsm.handle(e) || ...); },
                          hsms_);
    }

    std::tuple<Hsms...> hsms_;
};

// make orthogonal hsm from traits
template<typename... Ts>
struct make_orthogonal_hsm
{
    using type = OrthogonalHsm<convert_to_hsm_t<Ts>...>;
};

template<typename... Ts>
using make_orthogonal_hsm_t = typename make_orthogonal_hsm<Ts...>::type;
