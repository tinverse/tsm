#pragma once

#include "Event.h"
#include "EventQueue.h"
#include "ExecutionPolicy.h"

namespace tsm {
template<typename StateType>
struct ParentThreadExecutionPolicy
  : public ExecutionPolicy<StateType>
{
    using EventQueue = EventQueueT<Event, dummy_mutex>;

    ParentThreadExecutionPolicy() = delete;

    ParentThreadExecutionPolicy(StateType* sm)
      : ExecutionPolicy<StateType>(sm)
      , interrupt_(false)
    {}

    void start() override {}

    void stop() override { eventQueue_.stop(); }

    void step()
    {

        if (eventQueue_.empty()) {
            LOG(WARNING) << "Event Queue is empty!";
            return;
        }
        Event nextEvent;
        try {
            // This is a blocking wait
            nextEvent = eventQueue_.nextEvent();
        } catch (EventQueueInterruptedException const& e) {
            if (!interrupt_) {
                throw e;
            }
            LOG(WARNING) << this->sm_->name
                         << ": Exiting event loop on interrupt";
            return;
        }
        // go down the HSM hierarchy to handle the event as that is the
        // "most active state"
        this->sm_->dispatch(this->sm_)->execute(nextEvent);
    }

    void sendEvent(Event event) { eventQueue_.addEvent(event); }

  protected:
    EventQueue eventQueue_;
    bool interrupt_;
};
} // namespace tsm
