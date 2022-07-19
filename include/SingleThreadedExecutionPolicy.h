#pragma once

#include "Event.h"

#include <deque>

///
/// The policy for "synchronous" event processing. Events can be queued up in
/// the event queue as they arrive. However, to process each event, a
/// corresponding number of calls to the step function is required. So if there
/// are 3 queued events, the step function needs to be invoked 3 times for all
/// the events to be processed.
///
namespace tsm {
template<typename StateType>
struct SingleThreadedExecutionPolicy : public StateType
{
    ~SingleThreadedExecutionPolicy() = delete;
    using EventQueue = std::deque<Event>;

    void onExit(Event const& e) override { StateType::onExit(e); }

    void step()
    {
        // This is a blocking wait
        Event const& nextEvent = eventQueue_.front();
        eventQueue_.pop_front();
        // go down the Hsm hierarchy to handle the event as that is the
        // "most active state"
        StateType::dispatch(nextEvent);
    }

    void sendEvent(Event const& event) { eventQueue_.push_back(event); }

  private:
    EventQueue eventQueue_;
    bool interrupt_{};
};
} // namespace tsm
