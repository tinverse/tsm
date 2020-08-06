#pragma once

#include "Event.h"
#include "Hsm.h"
#include "State.h"

#include <memory>
#include <tuple>
#include <utility>

namespace tsm {
// See Fluent C++ for_each std tuple (https://www.fluentcpp.com)
template<class Tuple, class F, std::size_t... I>
constexpr F
for_each_impl(Tuple&& t, F&& f, std::index_sequence<I...>)
{
    return (void)std::initializer_list<int>{ (
             std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))), 0)... },
           f;
}

template<class Tuple, class F>
constexpr F
for_each_hsm(Tuple&& t, F&& f)
{
    return for_each_impl(
      std::forward<Tuple>(t),
      std::forward<F>(f),
      std::make_index_sequence<
        std::tuple_size<std::remove_reference_t<Tuple>>::value>{});
}

template<typename Tuple, typename Predicate>
constexpr size_t
find_if(Tuple&& tuple, Predicate pred)
{
    size_t index = std::tuple_size<std::remove_reference_t<Tuple>>::value;
    size_t currentIndex = 0;
    bool found = false;
    for_each_hsm(tuple, [&](auto&& value) {
        if (!found && pred(value)) {
            index = currentIndex;
            found = true;
        }
        ++currentIndex;
    });
    return index;
}

template<typename Tuple, typename Action>
void
perform(Tuple&& tuple, size_t index, Action action)
{
    size_t currentIndex = 0;
    for_each_hsm(
      tuple, [action = std::move(action), index, &currentIndex](auto&& value) {
          if (currentIndex == index) {
              action(std::forward<decltype(value)>(value));
          }
          ++currentIndex;
      });
}

template<typename... Hsms>
struct OrthogonalHsm
  : public IHsm
{
    using type = OrthogonalHsm<Hsms...>;
    static constexpr size_t HSM_COUNT = sizeof...(Hsms);

    OrthogonalHsm(std::string const& name)
      : IHsm(name, nullptr)
    {

        for_each_hsm(sms_, [&](auto& sm) { sm.setParent(this); });
    }

    OrthogonalHsm(std::string const& name, IHsm* parent)
      : IHsm(name, parent)
    {

        for_each_hsm(sms_, [&](auto& sm) { sm.setParent(this); });
    }

    void startSM() { onEntry(tsm::null_event); }

    void onEntry(Event const& e) override
    {
        DLOG(INFO) << "Entering: " << this->name;
        for_each_hsm(sms_, [&](auto& sm) { sm.onEntry(e); });
        this->setCurrentHsm(&std::get<0>(sms_));
    }

    void stopSM() { onExit(tsm::null_event); }

    void onExit(Event const& e) override
    {

        // Stopping a Hsm means stopping all of its sub Hsms
        for_each_hsm(sms_, [&](auto& sm) { sm.onExit(e); });

        this->setCurrentHsm(nullptr);
    }

    void handle(Event const& nextEvent) override
    {
        // Get the first hsm that handles the event
        auto sm_index = find_if(sms_, [&](auto& hsm) {
            auto supported_events = hsm.getEvents();
            auto event_it = supported_events.find(nextEvent);
            return (event_it != supported_events.end());
        });
        if (sm_index < HSM_COUNT) {
            perform(sms_, sm_index, [&](auto& sm) { sm.dispatch(nextEvent); });
        } else {
            // Try sending the event up to parent
            if (this->getParent()) {
                // Don't dispatch, directly handle here.
                this->getParent()->handle(nextEvent);
            } else {
                DLOG(ERROR) << "Reached top level Hsm. Cannot handle event";
            }
        }
    }

    State* getCurrentState()
    {
        return dynamic_cast<State*>(this->getCurrentHsm());
    }

    std::tuple<Hsms...> sms_;
};
} // namespace tsm
