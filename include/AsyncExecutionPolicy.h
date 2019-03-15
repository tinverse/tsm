#pragma once

#include "Event.h"
#include "EventQueue.h"

///
/// The default policy class for asynchronous event processing. This policy is
/// mixed in with a StateMachineT class to create an AsynchronousHsm. The
/// client uses the sendEvent method to communicate with the state machine. A
/// separate thread is created and blocks wating on events in the step method.
///
namespace tsm {
template<typename StateType>
struct AsyncExecutionPolicy : public StateType
{
    using EventQueue = EventQueueT<Event, std::mutex>;
    using ThreadCallback = void (AsyncExecutionPolicy::*)();

    AsyncExecutionPolicy()
      : StateType()
      , threadCallback_(&AsyncExecutionPolicy::step)
      , interrupt_(false)
    {}

    virtual ~AsyncExecutionPolicy() = default;

    void onEntry(Event const& e) override
    {
        StateType::onEntry(e);
        smThread_ = std::thread(threadCallback_, this);
    }

    void onExit(Event const& e) override
    {
        interrupt_ = true;
        eventQueue_.stop();
        smThread_.join();

        StateType::onExit(e);
    }

    virtual void step()
    {
        while (!interrupt_) {
            processEvent();
        }
    };

    void sendEvent(Event const& event) { eventQueue_.addEvent(event); }

  protected:
    ThreadCallback threadCallback_;
    std::thread smThread_;
    EventQueue eventQueue_;
    bool interrupt_;

    void processEvent()
    {
        // This is a blocking wait
        Event const& nextEvent = eventQueue_.nextEvent();
        // go down the Hsm hierarchy to handle the event as that is the
        // "most active state"
        if (!eventQueue_.interrupted()) {
            this->dispatch(this)->execute(nextEvent);
        } else {
            DLOG(WARNING) << this->name << ": Exiting event loop on interrupt";
        }
    }
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
    using Observer::notify;

    AsyncExecWithObserver()
      : AsyncExecutionPolicy<StateType>()
      , Observer()
    {}

    virtual ~AsyncExecWithObserver() = default;

    void step() override
    {
        while (!interrupt_) {
            notify();
            this->processEvent();
        }
    }
};
} // namespace tsm
