#pragma once

#include "tsm_log.h"

#ifdef __FREE_RTOS__
#include "FreeRTOS.h"
#include "semphr.h"
#include "event_groups.h"
#else
#include <condition_variable>
#include <deque>
#include <iostream>
#include <mutex>
#include <string>

using std::deque;
#endif // __FREE_RTOS__


// Customize for Free RTOS
namespace tsm {

// A thread safe event queue. Any thread can call addEvent if it has a pointer
// to the event queue. The call to nextEvent is a blocking call

#ifdef __FREE_RTOS__
constexpr int MaxEvents = 10; // Define your own queue size

class EventQueue {
private:
    Event events[MaxEvents];
    int front = 0, rear = 0;
    SemaphoreHandle_t mutex;
    EventGroupHandle_t eventGroup;
    BaseType_t interrupt = pdFALSE;

public:
    EventQueue() {
        mutex = xSemaphoreCreateMutex();
        eventGroup = xEventGroupCreate();
    }

    ~EventQueue() {
        vSemaphoreDelete(mutex);
        vEventGroupDelete(eventGroup);
    }

    void addEvent(const Event& e) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        if ((rear + 1) % MaxEvents != front) { // Check space
            events[rear] = e;
            rear = (rear + 1) % MaxEvents;
            xEventGroupSetBits(eventGroup, 0x01); // Set event bit
        }
        xSemaphoreGive(mutex);
    }

    Event nextEvent() {
        Event e;
        while (true) {
            xEventGroupWaitBits(eventGroup, 0x01, pdTRUE, pdFALSE, portMAX_DELAY); // Wait for an event
            xSemaphoreTake(mutex, portMAX_DELAY);
            if (interrupt) {
                xSemaphoreGive(mutex);
                return {}; // Return default-constructed Event
            }
            if (front != rear) { // Check if there is an event
                e = events[front];
                front = (front + 1) % MaxEvents;
                xSemaphoreGive(mutex);
                return e;
            }
            xSemaphoreGive(mutex);
        }
    }

    void stop() {
        xSemaphoreTake(mutex, portMAX_DELAY);
        interrupt = pdTRUE;
        xEventGroupSetBits(eventGroup, 0x01); // Wake up waiting tasks
        xSemaphoreGive(mutex);
    }

    void addFront(const Event& e) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        front = (front - 1 + MaxEvents) % MaxEvents; // Move front backwards
        if (front != rear) { // Check space
            events[front] = e;
            xEventGroupSetBits(eventGroup, 0x01); // Set event bit
        } else {
            // Queue is full, move front back to original position
            front = (front + 1) % MaxEvents;
        }
        xSemaphoreGive(mutex);
    }
};

#else

template<typename Event, typename LockType>
struct EventQueueT : private deque<Event>
{
    using deque<Event>::empty;
    using deque<Event>::front;
    using deque<Event>::pop_front;
    using deque<Event>::push_back;
    using deque<Event>::push_front;
    using deque<Event>::size;

  public:
    EventQueueT() = default;
    EventQueueT(EventQueueT const&) = delete;
    EventQueueT(EventQueueT&&) = delete;
    EventQueueT operator=(EventQueueT const&) = delete;
    EventQueueT operator=(EventQueueT&&) = delete;

    ~EventQueueT() { stop(); }

    // Block until you get an event
    Event nextEvent()
    {
        std::unique_lock<LockType> lock(eventQueueMutex_);
        cvEventAvailable_.wait(
          lock, [this] { return (!this->empty() || this->interrupt_); });
        if (interrupt_) {
            return Event();
        }
        const Event e = std::move(front());
        // LOG(INFO) << "Thread:" << std::this_thread::get_id()
        //          << " Popping Event:" << e.id;
        pop_front();
        return e;
    }

    void addEvent(Event const& e)
    {
        std::lock_guard<LockType> lock(eventQueueMutex_);
        // LOG(INFO) << "Thread:" << std::this_thread::get_id()
        //          << " Adding Event:" << e.id;
        push_back(e);
        cvEventAvailable_.notify_all();
    }

    void stop()
    {
        interrupt_ = true;
        cvEventAvailable_.notify_all();
        // Log the events that are going to get dumped if the queue is not
        // empty
    }

    bool interrupted() { return interrupt_; }

    void addFront(Event const& e)
    {
        std::lock_guard<LockType> lock(eventQueueMutex_);
        push_front(e);
        cvEventAvailable_.notify_all();
    }

  private:
    LockType eventQueueMutex_;
    std::condition_variable_any cvEventAvailable_;
    bool interrupt_{};
};

template<typename Event>
using EventQueue = EventQueueT<Event, std::mutex>;
#endif
} // namespace tsm
