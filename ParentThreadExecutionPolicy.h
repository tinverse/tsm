#pragma once

#include "Event.h"
#include "EventQueue.h"
#include "ExecutionPolicy.h"
///
/// The policy for "synchronous" event processing. Events can be queued up in
/// the event queue as they arrive. However, to process each event, a
/// corresponding number of calls to the step function is required. So if there
/// are 3 queued events, the step function needs to be invoked 3 times for all
/// the events to be processed.
///
namespace tsm {
template<typename StateType>
struct ParentThreadExecutionPolicy : public ExecutionPolicy<StateType>
{
    using EventQueue = EventQueueT<Event, dummy_mutex>;

    ParentThreadExecutionPolicy() = delete;

    ParentThreadExecutionPolicy(StateType* sm)
      : ExecutionPolicy<StateType>(sm)
      , interrupt_(false)
    {}

    void entry() override {}

    void exit() override { eventQueue_.stop(); }

    void step()
    {

        if (eventQueue_.empty()) {
            LOG(WARNING) << "Event Queue is empty!";
            return;
        }
        try {
            // This is a blocking wait
            Event const& nextEvent = eventQueue_.nextEvent();
            // go down the HSM hierarchy to handle the event as that is the
            // "most active state"
            this->sm_->dispatch(this->sm_)->execute(nextEvent);

        } catch (EventQueueInterruptedException const& e) {
            if (!interrupt_) {
                throw e;
            }
            LOG(WARNING) << this->sm_->name
                         << ": Exiting event loop on interrupt";
            return;
        }
    }

    void sendEvent(Event const& event) { eventQueue_.addEvent(event); }

  protected:
    EventQueue eventQueue_;
    bool interrupt_;
};
} // namespace tsm
