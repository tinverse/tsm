#pragma once

#include "Event.h"
#include "EventQueue.h"
#include "ExecutionPolicy.h"
///
/// The default policy class for asynchronous event processing. This policy is
/// mixed in with a StateMachineT class to create an AsyncStateMachine. The
/// client uses the sendEvent method to communicate with the state machine. A
/// separate thread is created and blocks wating on events in the step method.
///
namespace tsm {
template<typename StateType>
struct AsyncExecutionPolicy : public ExecutionPolicy<StateType>
{
    using EventQueue = EventQueueT<Event, std::mutex>;
    using ThreadCallback = void (AsyncExecutionPolicy::*)();

    AsyncExecutionPolicy() = delete;

    AsyncExecutionPolicy(StateType* sm)
      : ExecutionPolicy<StateType>(sm)
      , threadCallback_(&AsyncExecutionPolicy::step)
      , interrupt_(false)
    {}

    void entry() override { smThread_ = std::thread(threadCallback_, this); }

    void exit() override
    {
        interrupt_ = true;
        eventQueue_.stop();
        smThread_.join();
    }

    virtual void step()
    {
        while (!interrupt_) {
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
                LOG(WARNING)
                  << this->sm_->name << ": Exiting event loop on interrupt";
                return;
            }
        }
    };

    void sendEvent(Event const& event) { eventQueue_.addEvent(event); }

  protected:
    ThreadCallback threadCallback_;
    std::thread smThread_;
    EventQueue eventQueue_;
    bool interrupt_;
};

///
/// Another asynchronous state machine execution policy. The only difference is
/// that an Observer's notify method will be invoked at the end of processing
/// each event - specifically, right before the blocking wait for the next
/// event.
///
template<typename StateType, typename Observer>
struct AsyncExecWithObserver
  : public AsyncExecutionPolicy<StateType>
  , public Observer
{
    using AsyncExecutionPolicy<StateType>::interrupt_;
    using AsyncExecutionPolicy<StateType>::eventQueue_;
    using Observer::notify;

    AsyncExecWithObserver() = delete;

    AsyncExecWithObserver(StateType* sm)
      : AsyncExecutionPolicy<StateType>(sm)
      , Observer()
    {}

    void step() override
    {
        while (!interrupt_) {
            try {
                notify();
                // This is a blocking wait
                Event const& nextEvent = eventQueue_.nextEvent();
                // go down the HSM hierarchy to handle the event as that is the
                // "most active state"
                this->sm_->dispatch(this->sm_)->execute(nextEvent);

            } catch (EventQueueInterruptedException const& e) {
                if (!interrupt_) {
                    throw e;
                }
                LOG(WARNING)
                  << this->sm_->name << ": Exiting event loop on interrupt";
                return;
            }
        }
    };
};
} // namespace tsm
