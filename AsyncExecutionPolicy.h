#pragma once

#include "Event.h"
#include "EventQueue.h"
#include "ExecutionPolicy.h"

namespace tsm {
template<typename StateType>
struct AsyncExecutionPolicy
  : public ExecutionPolicy<StateType>
{
    using EventQueue = EventQueueT<Event, std::mutex>;
    using ThreadCallback = void (AsyncExecutionPolicy::*)();

    AsyncExecutionPolicy() = delete;

    AsyncExecutionPolicy(StateType* sm)
      : ExecutionPolicy<StateType>(sm)
      , threadCallback_(&AsyncExecutionPolicy::step)
      , interrupt_(false)
    {}

    void start() override { smThread_ = std::thread(threadCallback_, this); }

    void stop() override
    {
        interrupt_ = true;
        eventQueue_.stop();
        smThread_.join();
    }

    void step()
    {
        while (!interrupt_) {
            Event nextEvent;
            try {
                // This is a blocking wait
                nextEvent = eventQueue_.nextEvent();
            } catch (EventQueueInterruptedException const& e) {
                if (!interrupt_) {
                    throw e;
                }
                LOG(WARNING)
                  << this->sm_->name << ": Exiting event loop on interrupt";
                return;
            }
            // go down the HSM hierarchy to handle the event as that is the
            // "most active state"
            // this->sm_->dispatch(this->sm_)->execute(nextEvent);
            this->sm_->dispatch(this->sm_)->execute(nextEvent);
        }
    };

    void sendEvent(Event event) { eventQueue_.addEvent(event); }

  protected:
    ThreadCallback threadCallback_;
    std::thread smThread_;
    EventQueue eventQueue_;
    bool interrupt_;
};
} // namespace tsm
