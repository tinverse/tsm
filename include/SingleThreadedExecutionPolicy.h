#pragma once

#include "Event.h"
#include "EventQueue.h"
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
    using EventQueue = EventQueueT<Event, null_mutex>;

    SingleThreadedExecutionPolicy()
      : StateType()
      , interrupt_(false)
    {}

    virtual ~SingleThreadedExecutionPolicy() = default;

    void onExit(Event const& e) override
    {
        DLOG(INFO) << "Exiting from Parent thread policy...";
        eventQueue_.stop();
        StateType::onExit(e);
    }

    void step()
    {
        if (eventQueue_.empty()) {
            DLOG(WARNING) << "Event Queue is empty!";
            return;
        }
        // This is a blocking wait
        Event const& nextEvent = eventQueue_.nextEvent();
        // go down the Hsm hierarchy to handle the event as that is the
        // "most active state"
        if (!eventQueue_.interrupted()) {
            this->dispatch()->execute(nextEvent);
        } else {
            DLOG(WARNING) << this->name << ": Exiting event loop on interrupt";
        }
    }

    void sendEvent(Event const& event) { eventQueue_.addEvent(event); }

  protected:
    EventQueue eventQueue_;
    bool interrupt_;
};
} // namespace tsm
