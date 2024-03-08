#pragma once

#include "Event.h"
#include "EventQueue.h"
#ifdef __FREE_RTOS__
#include "FreeRTOS.h"
#include "task.h"
#else
#include <thread>
#endif
///
/// The default policy class for asynchronous event processing. This policy is
/// mixed in with a Hsm class to create an AsynchronousHsm. The client uses
/// the sendEvent method to communicate with the state machine. A separate
/// thread is created and blocks wating on events in the step method.
///
namespace tsm {
#ifdef __FREE_RTOS__

template<typename StateType>
class AsyncExecutionPolicy : public StateType {
public:
    // using EventQueue = FreeRTOSEventQueue<Event>; // Adapted for FreeRTOS
    using TaskCallback = void (*)(AsyncExecutionPolicy*);

    AsyncExecutionPolicy() : taskCallback(AsyncExecutionPolicy::StepTask) {
        interrupt_ = pdFALSE;
    }

    AsyncExecutionPolicy(const AsyncExecutionPolicy&) = delete;
    AsyncExecutionPolicy& operator=(const AsyncExecutionPolicy&) = delete;
    AsyncExecutionPolicy(AsyncExecutionPolicy&&) = delete;
    AsyncExecutionPolicy& operator=(AsyncExecutionPolicy&&) = delete;

    virtual ~AsyncExecutionPolicy() {
        interrupt_ = pdTRUE;
        // Proper FreeRTOS task deletion if needed, ensuring clean-up.
    }

    void onEntry(const Event& e) override {
        StateType::onEntry(e);
        // Create a FreeRTOS task instead of a C++ thread
        xTaskCreate(reinterpret_cast<TaskFunction_t>(taskCallback), "AsyncPolicyTask", configMINIMAL_STACK_SIZE, this, tskIDLE_PRIORITY, &smTaskHandle);
    }

    void onExit(const Event& e) override {
        eventQueue.stop();
        interrupt_ = pdTRUE;
        if (smTaskHandle != NULL) {
            vTaskDelete(smTaskHandle);
            smTaskHandle = NULL;
        }
        StateType::onExit(e);
    }

    void sendEvent(const Event& event) {
        eventQueue.addEvent(event);
    }

protected:
    TaskCallback taskCallback;
    TaskHandle_t smTaskHandle = NULL;
    EventQueue eventQueue;
    BaseType_t interrupt_;

    static void StepTask(void* pvParameters) {
        auto* policy = static_cast<AsyncExecutionPolicy*>(pvParameters);
        policy->step();
    }

    void step() {
        while (!interrupt_) {
            processEvent();
        }
    }

    void processEvent() {
        Event const& nextEvent = eventQueue.nextEvent();
        if (!eventQueue.interrupted()) {
            StateType::dispatch(nextEvent);
        } else {
            // Replace with FreeRTOS logging mechanism or custom logger
            // For example, printf("ID %d: Exiting event loop on interrupt", this->id);
        }
    }
};
#else
template<typename StateType>
struct AsyncExecutionPolicy : public StateType
{
    using EventQueue = EventQueueT<Event, std::mutex>;
    using ThreadCallback = void (AsyncExecutionPolicy::*)();

    AsyncExecutionPolicy()
      : threadCallback_(&AsyncExecutionPolicy::step)
    {}

    AsyncExecutionPolicy(AsyncExecutionPolicy const&) = delete;
    AsyncExecutionPolicy operator=(AsyncExecutionPolicy const&) = delete;
    AsyncExecutionPolicy(AsyncExecutionPolicy&&) = delete;
    AsyncExecutionPolicy operator=(AsyncExecutionPolicy&&) = delete;

    virtual ~AsyncExecutionPolicy()
    {
        interrupt_ = true;
        if (smThread_.joinable()) {
            smThread_.join();
        }
    }

    void onEntry(Event const& e) override
    {
        StateType::onEntry(e);
        smThread_ = std::thread(threadCallback_, this);
    }

    void onExit(Event const& e) override
    {
        eventQueue_.stop();
        interrupt_ = true;

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
    bool interrupt_{};

    void processEvent()
    {
        // This is a blocking wait
        Event const& nextEvent = eventQueue_.nextEvent();
        // go down the Hsm hierarchy to handle the event as that is the
        // "most active state"
        if (!eventQueue_.interrupted()) {
            StateType::dispatch(nextEvent);
        } else {
            LOG(WARNING) << this->id << ": Exiting event loop on interrupt";
        }
    }
};
#endif
///
/// Another asynchronous execution policy. The only difference with above is
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

    void step() override
    {
        while (!interrupt_) {
            notify();
            this->processEvent();
        }
    }
};
} // namespace tsm
